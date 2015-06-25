using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using OWF.DTO;
using System.IO;

namespace OWF.Serializers
{
    public static class BinaryDeserializer
    {
        public static string readOWFString(BinaryReader br)
        {
            checked
            {
                UInt32 strlen = readU32(br);
                byte[] rawString = br.ReadBytes( (int) strlen); // please no gb strings
                return Encoding.UTF8.GetString(rawString, 0, rawString.Length);
            }
        }

        public static DateTime readOWFTimestamp(BinaryReader br)
        {
            Int64 rawTime = readS64(br);
            // filetime is 100ns increments from 1601...we use a different epoch, so we offset.
            Int64 unixEpochOffset = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc).ToFileTime();
            Int64 adjTime = rawTime + unixEpochOffset;
            return new DateTime(adjTime);                        
        }

        public static Alarm readAlarm(BinaryReader br)
        {
            DateTime time = readOWFTimestamp(br);
            TimeSpan duration = new TimeSpan(checked((long)readU64(br)));
            byte level = br.ReadByte();
            byte volume = br.ReadByte();
            br.ReadBytes(2); // skip padding

            string msgType = readOWFString(br);
            string message = readOWFString(br);

            return new Alarm(time, duration, level, volume, msgType, message);
        }

        public static Event readEvent(BinaryReader br)
        {
            DateTime time = readOWFTimestamp(br);
            string data = readOWFString(br);
            return new Event(time, data);
        }

        public static Signal readSignal( BinaryReader br)
        {
            string id = readOWFString(br);
            string unit = readOWFString(br);
            UInt32 samplesLength = readU32(br);

            if (samplesLength % sizeof(double) != 0)
            {
                throw new Exception("Encountered wrong size samples array");
            }

            byte[] rawSamples = br.ReadBytes((int) samplesLength); //please no gb of doubles :(
            double[] samples = new double[samplesLength/8];
            Buffer.BlockCopy(rawSamples, 0, samples, 0, (int) samplesLength);
            if (BitConverter.IsLittleEndian)
            {
                for (var i = 0; i < samples.Length; i++)
                {
                    byte[] valueBytes = BitConverter.GetBytes(samples[i]);
                    Array.Reverse(valueBytes);
                    samples[i] = BitConverter.ToDouble(valueBytes,0);
                }
            }
            else
            {
                // do nothing
            }

            return new Signal(id, unit, samples);
        }

        public static Namespace readNamespace(MemoryStream ms)
        {
            List<Signal> signals = new List<Signal>();
            List<Alarm> alarms = new List<Alarm>();
            List<Event> events = new List<Event>();
            DateTime t0;
            TimeSpan dt;
            string id;
            using (BinaryReader br = new BinaryReader(ms, Encoding.UTF8))
            {                
                UInt32 length = readU32(br);
                UInt32 offset;
                using (MemoryStream namespaceStream = new MemoryStream(ms.GetBuffer(), (int) ms.Position, (int)length, false))
                {
                    t0 = readOWFTimestamp(br);
                    dt = new TimeSpan(checked((long)readU64(br)));
                    id = readOWFString(br);

                    UInt32 signalsLength = readU32(br);
                    offset = (UInt32)namespaceStream.Position;
                    using (MemoryStream signalStream = new MemoryStream(ms.GetBuffer(), (int)ms.Position, (int)signalsLength, false))
                    {
                        while (offset < signalsLength)
                        {
                            using (BinaryReader sbr = new BinaryReader(signalStream, Encoding.UTF8))
                            {                                
                                signals.Add(readSignal(sbr));                                
                            }
                            // no seek...readSignal advances stream for us
                        }
                    }
                    namespaceStream.Seek(signalsLength, SeekOrigin.Current); ;

                    UInt32 eventsLength = readU32(br);
                    offset = (UInt32) namespaceStream.Position;
                    using (MemoryStream eventsStream = new MemoryStream(ms.GetBuffer(), (int)ms.Position, (int)eventsLength, false))
                    {
                        while( offset < eventsStream.Length)
                        {
                            using( BinaryReader ebr = new BinaryReader(eventsStream, Encoding.UTF8) )
                            {
                                events.Add( readEvent(ebr));
                            }
                            // no seeking because readEvent moves the stream for us
                        }
                    }
                    namespaceStream.Seek(eventsLength, SeekOrigin.Current); ;

                    UInt32 alarmsLength = readU32(br);
                    offset = (UInt32)namespaceStream.Position;
                    using (MemoryStream alarmsStream = new MemoryStream(ms.GetBuffer(), (int)ms.Position, (int)alarmsLength, false))
                    {
                        while (offset < alarmsStream.Length)
                        {
                            using (BinaryReader abr = new BinaryReader(alarmsStream, Encoding.UTF8))
                            {
                                alarms.Add(readAlarm(abr));
                            }
                            // no seeking because readAlarm moves the stream for us
                        }
                    }
                    namespaceStream.Seek(alarmsLength, SeekOrigin.Current);
                }
                ms.Seek(length, SeekOrigin.Current);
                return new Namespace(id, t0, dt, signals, events, alarms);
            }
        }

        public static Channel readChannel(MemoryStream ms)
        {
            using (BinaryReader br = new BinaryReader(ms, Encoding.UTF8))
            {
                List<Namespace> namespaces = new List<Namespace>();
                UInt32 length = readU32(br);
                string id = readOWFString(br);                

                UInt32 offset = (UInt32)ms.Position;
                while (offset < length)
                {
                    UInt32 namespaceLength;
                    namespaceLength = readU32(br);
                    using (MemoryStream namespaceStream = new MemoryStream(ms.GetBuffer(), (int)offset, (int)namespaceLength, false))
                    {
                        Namespace ns = readNamespace(namespaceStream);
                        namespaces.Add(ns);
                    }
                    offset += namespaceLength;
                    ms.Seek(namespaceLength, SeekOrigin.Current);
                }

                return new Channel(id, namespaces);
            }
        }

        public static UInt32 readU32(BinaryReader br)
        {
            UInt32 val = br.ReadUInt32();
            if (BitConverter.IsLittleEndian)
            {                
                byte[] valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                val = BitConverter.ToUInt32(valueBytes, 0);
            }
            else
            {
                // do nothing
            }

            return val;
        }

        public static UInt64 readU64(BinaryReader br)
        {
            UInt64 val = br.ReadUInt64();
            if (BitConverter.IsLittleEndian)
            {
                byte[] valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                val = BitConverter.ToUInt32(valueBytes, 0);
            }
            else
            {
                // do nothing
            }

            return val;
        }

        public static Int64 readS64(BinaryReader br)
        {
            Int64 val = br.ReadInt64();
            if (BitConverter.IsLittleEndian)
            {
                byte[] valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                val = BitConverter.ToUInt32(valueBytes, 0);
            }
            else
            {
                // do nothing
            }

            return val;
        }

        public static UInt32 readHeader(BinaryReader br)
        {
            byte[] magic = { 0x4f, 0x57, 0x46, 0x31 };
            byte[] rawMagic = br.ReadBytes(4);
            if (rawMagic.SequenceEqual(magic) == false)
            {
                throw new Exception("Bad header magic number");
            }

            return readU32(br);
        }
        
        public static Package convert(byte[] package)
        {
            checked
            {
                using (MemoryStream mem = new MemoryStream(package, false))
                {
                    UInt32 length = 0;
                    using (BinaryReader br = new BinaryReader(mem, Encoding.UTF8)) 
                    {                        
                        length = readHeader(br);
                    }

                    UInt32 offset = (UInt32)mem.Position;
                    List<Channel> channels = new List<Channel>();                                        
                    while (offset < length)
                    {
                        UInt32 channelLength;
                        using (BinaryReader br = new BinaryReader(mem, Encoding.UTF8))
                        {
                            channelLength = readU32(br);
                        }
                                                
                        using (MemoryStream channelStream = new MemoryStream(mem.GetBuffer(), (int)offset, (int)channelLength, false))
                        {
                            Channel channel = readChannel(channelStream);
                            channels.Add(channel);
                        }
                        offset += channelLength;                        
                    }
                    
                    return new Package(channels);                        
                }
            }
        }
    }
}
