using System;
using System.IO;
using System.Linq;
using System.Text;
using OWF.DTO;

namespace OWF.Serializers {
    public static class BinarySerializer {
        public static void WriteOWFString(BinaryWriter bw, OWFString str) {
            checked {
                var fullSize = str.GetSizeInBytes() - sizeof(UInt32);

                // write length
                WriteU32(bw, fullSize);

                // write string and null-terminator
                if (fullSize > 0) {
                    // Get the bytes, and write them, adding a null-terminator
                    byte[] bytes = Encoding.UTF8.GetBytes(str.Value);
                    bw.Write(bytes);
                    bw.Write((byte)0);

                    // Subtract the length we just wrote
                    fullSize -= (UInt32)(bytes.Length + 1);
                }

                // write any padding
                while (fullSize > 0) {
                    bw.Write((byte)0);
                    fullSize--;
                }
            }
        }

        public static void WriteOWFDoubles(BinaryWriter bw, double[] values) {
            if (BitConverter.IsLittleEndian) {
                foreach (var valueBytes in values.Select(BitConverter.GetBytes)) {
                    Array.Reverse(valueBytes);
                    bw.Write(valueBytes);
                }
            }
            else {
                foreach (var v in values) {
                    bw.Write(v);
                }
            }
        }

        public static void WriteU64(BinaryWriter bw, UInt64 val) {
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                bw.Write(valueBytes);
            }
            else {
                bw.Write(val);
            }
        }

        public static void WriteS64(BinaryWriter bw, Int64 val) {
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                bw.Write(valueBytes);
            }
            else {
                bw.Write(val);
            }
        }

        public static void WriteU32(BinaryWriter bw, UInt32 val) {
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                bw.Write(valueBytes);
            }
            else {
                bw.Write(val);
            }
        }

        public static void WriteDouble(BinaryWriter bw, double val) {
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                bw.Write(valueBytes);
            }
            else {
                bw.Write(val);
            }
        }

        public static void WriteHeader(BinaryWriter bw, UInt32 length) {
            bw.Write(OWFObject.Magic);
            WriteU32(bw, length);
        }

        public static void WriteChannel(BinaryWriter bw, OWFChannel channel) {
            WriteU32(bw, checked(channel.GetSizeInBytes() - sizeof(UInt32)));
            WriteOWFString(bw, channel.Id);
            foreach (var ns in channel.Namespaces) {
                WriteNamespace(bw, ns);
            }
        }

        public static void WriteNamespace(BinaryWriter bw, OWFNamespace ns) {
            WriteU32(bw, checked(ns.GetSizeInBytes() - sizeof(UInt32)));
            WriteS64(bw, ns.T0);
            WriteU64(bw, ns.Dt);
            WriteOWFString(bw, ns.Id);

            UInt32 signalsLength = ns.Signals.Aggregate(0U, (current, sig) => checked(current + sig.GetSizeInBytes()));
            UInt32 eventsLength = ns.Events.Aggregate(0U, (current, evt) => checked(current + evt.GetSizeInBytes()));
            UInt32 alarmsLength = ns.Alarms.Aggregate(0U, (current, alarm) => checked(current + alarm.GetSizeInBytes()));

            WriteU32(bw, signalsLength);
            foreach (var sig in ns.Signals) {
                WriteSignal(bw, sig);
            }

            WriteU32(bw, eventsLength);
            foreach (var evt in ns.Events) {
                WriteEvent(bw, evt);
            }

            WriteU32(bw, alarmsLength);
            foreach (var alarm in ns.Alarms) {
                WriteAlarm(bw, alarm);
            }
        }

        public static void WriteSignal(BinaryWriter bw, OWFSignal sig) {
            WriteOWFString(bw, sig.Id);
            WriteOWFString(bw, sig.Unit);

            UInt32 samplesLength = checked(sizeof(double) * (UInt32)sig.Samples.Length);
            WriteU32(bw, samplesLength);
            WriteOWFDoubles(bw, sig.Samples);
        }

        public static void WriteAlarm(BinaryWriter bw, OWFAlarm alarm) {
            WriteS64(bw, alarm.T0);
            WriteU64(bw, (UInt64)alarm.TimeSpan.Ticks);
            bw.Write(alarm.Level);
            bw.Write(alarm.Volume);
            bw.Write((UInt16)0);
            WriteOWFString(bw, alarm.Type);
            WriteOWFString(bw, alarm.Message);
        }

        public static void WriteEvent(BinaryWriter bw, OWFEvent evt) {
            WriteS64(bw, evt.T0);
            WriteOWFString(bw, evt.Message);
        }

        public static byte[] Convert(OWFPackage package) {
            using (var mem = new MemoryStream()) {
                using (var bw = new BinaryWriter(mem, Encoding.UTF8)) {
                    WriteHeader(bw, checked(package.GetSizeInBytes() - sizeof(UInt32)));

                    foreach (var channel in package.Channels) {
                        WriteChannel(bw, channel);
                    }
                }
                return mem.ToArray();
            }
        }
    }
}