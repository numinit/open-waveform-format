using System;
using System.Collections.Generic;
using System.Text;
using OWF.DTO;
using System.IO;

namespace OWF.Serializers
{
    public static class BinarySerializer
    {        

        public static void writeOWFString(BinaryWriter bw, string str)
        {
            checked
            {
                UInt32 strsize = (UInt32)(System.Text.Encoding.UTF8.GetByteCount(str));
                UInt32 padding = strsize % 4;

                // write length
                UInt32 fullSize = checked((strsize + padding));
                writeU32(bw, fullSize);


                // write string (this works becuase the Binary Writer is UTF8 encoded)
                bw.Write(str);

                // write any padding
                while (padding > 0)
                {
                    bw.Write( (byte) 0 );
                    padding--;
                }
            }
        }

        public static void writeOWFDoubles( BinaryWriter bw, double [] values )
        {            
            if (BitConverter.IsLittleEndian)
            {
                for (var i = 0; i < values.Length; i++)
                {
                    byte[] valueBytes = BitConverter.GetBytes(values[i]);
                    Array.Reverse(valueBytes);
                    bw.Write(valueBytes);
                }
            }
            else
            {
                for (var i = 0; i < values.Length; i++)
                {
                    bw.Write(values[i]);
                }
            }
        }

        public static void writeOWFTimestamp(BinaryWriter bw, DateTime time)
        {
            // filetime is 100ns increments from 1601...we use a different epoch, so we offset.
            Int64 unixEpochOffset = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc).ToFileTime();
            Int64 time64 = time.ToFileTime();
            Int64 adjTime = time64 - unixEpochOffset;

            writeS64(bw, adjTime);
        }

        public static void writeU64(BinaryWriter bw, UInt64 val)
        {
            if (BitConverter.IsLittleEndian)
            {
                byte[] valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                bw.Write(valueBytes);
            }
            else
            {
                bw.Write(val);
            }
        }

        public static void writeS64(BinaryWriter bw, Int64 val)
        {
            if (BitConverter.IsLittleEndian)
            {
                byte[] valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                bw.Write(valueBytes);
            }
            else
            {
                bw.Write(val);
            }
        }

        public static void writeU32(BinaryWriter bw, UInt32 val)
        {
            if (BitConverter.IsLittleEndian)
            {
                byte[] valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                bw.Write(valueBytes);
            }
            else
            {
                bw.Write(val);
            }
        }

        public static void writeDouble(BinaryWriter bw, double val)
        {
            if (BitConverter.IsLittleEndian)
            {
                byte[] valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                bw.Write(valueBytes);
            }
            else
            {
                bw.Write(val);
            }
        }

        public static void writeHeader( BinaryWriter bw, UInt32 length ){
            byte[] magic = {0x4f, 0x57, 0x46, 0x31 };
            bw.Write( magic );
            writeU32(bw, length);            
        }

        public static void writeChannel(BinaryWriter bw, Channel channel)
        {
            writeU32(bw, channel.getSizeInBytes());
            writeOWFString(bw, channel.Id);
            foreach (var ns in channel.Namespaces)
            {
                writeNamespace(bw, ns);
            }
        }

        public static void writeNamespace(BinaryWriter bw, Namespace ns)
        {
            writeU32(bw, ns.getSizeInBytes());
            writeOWFTimestamp(bw, ns.t0);
            writeU64(bw, (UInt64) ns.dt.Ticks);
            writeOWFString(bw, ns.Id);

            UInt32 alarmsLength = 0;
            UInt32 eventsLength = 0;
            UInt32 signalsLength = 0;
            checked
            {
                foreach (var sig in ns.Signals)
                {
                    signalsLength += sig.getSizeInBytes();
                }

                foreach (var alarm in ns.Alarms)
                {
                    alarmsLength += alarm.getSizeInBytes();
                }

                foreach (var evt in ns.Events)
                {
                    eventsLength += evt.getSizeInBytes();
                }
            }

            writeU32(bw, signalsLength);
            foreach (var sig in ns.Signals)
            {
                writeSignal(bw, sig);
            }

            writeU32(bw, alarmsLength);
            foreach (var alarm in ns.Alarms)
            {
                writeAlarm(bw, alarm);
            }

            writeU32(bw, eventsLength);
            foreach (var evt in ns.Events)
            {
                writeEvent(bw, evt);
            }
        }

        public static void writeSignal(BinaryWriter bw, Signal sig)
        {
            writeU32(bw, sig.getSizeInBytes());
            writeOWFString(bw, sig.Id);
            writeOWFString(bw, sig.Unit);

            UInt32 samplesLength = 0;
            checked
            {
                samplesLength = sizeof(double) * (UInt32) sig.Samples.Length;
            }
            writeU32(bw, samplesLength);
            writeOWFDoubles(bw, sig.Samples);
        }

        public static void writeAlarm(BinaryWriter bw, Alarm alarm)
        {
            writeOWFTimestamp(bw, alarm.Time);
            writeOWFString(bw, alarm.Data);
        }

        public static void writeEvent(BinaryWriter bw, Event evt)
        {
            writeOWFTimestamp(bw, evt.Time);
            writeOWFString(bw, evt.Data);
        }

        public static byte[] convert(Package package)
        {
            using (MemoryStream mem = new MemoryStream())
            {
                using (BinaryWriter bw = new BinaryWriter(mem, Encoding.UTF8))
                {
                    writeHeader(bw, package.getSizeInBytes());

                    foreach( var channel in package.Channels) {
                        writeChannel(bw, channel);
                    }
                }
                return mem.ToArray();
            }            
        }
    }
}
