using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using OWF.DTO;

namespace OWF.Serializers {
    public static class BinaryDeserializer {
        public static string ReadOWFString(BinaryReader br) {
            checked {
                var strlen = ReadU32(br);
                var rawString = br.ReadBytes((int)strlen); // please no gb strings
                return Encoding.UTF8.GetString(rawString, 0, rawString.Length);
            }
        }

        public static DateTime ReadOWFTimestamp(BinaryReader br) {
            var rawTime = ReadS64(br);
            // filetime is 100ns increments from 1601...we use a different epoch, so we offset.
            var unixEpochOffset = new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc).ToFileTime();
            var adjTime = rawTime + unixEpochOffset;
            return new DateTime(adjTime);
        }

        public static OWFAlarm ReadAlarm(BinaryReader br) {
            var time = ReadOWFTimestamp(br);
            var duration = new TimeSpan(checked((long)ReadU64(br)));
            var level = br.ReadByte();
            var volume = br.ReadByte();
            br.ReadBytes(2); // skip padding

            var msgType = ReadOWFString(br);
            var message = ReadOWFString(br);

            return new OWFAlarm(time, duration, level, volume, msgType, message);
        }

        public static OWFEvent ReadEvent(BinaryReader br) {
            var time = ReadOWFTimestamp(br);
            var data = ReadOWFString(br);
            return new OWFEvent(time, data);
        }

        public static OWFSignal ReadSignal(BinaryReader br) {
            var id = ReadOWFString(br);
            var unit = ReadOWFString(br);
            var samplesLength = ReadU32(br);

            if (samplesLength % sizeof(double) != 0) {
                throw new Exception("Encountered wrong size samples array");
            }

            var rawSamples = br.ReadBytes((int)samplesLength); //please no gb of doubles :(
            var samples = new double[samplesLength / 8];
            Buffer.BlockCopy(rawSamples, 0, samples, 0, (int)samplesLength);
            if (BitConverter.IsLittleEndian) {
                for (var i = 0; i < samples.Length; i++) {
                    var valueBytes = BitConverter.GetBytes(samples[i]);
                    Array.Reverse(valueBytes);
                    samples[i] = BitConverter.ToDouble(valueBytes, 0);
                }
            }

            return new OWFSignal(id, unit, samples);
        }

        public static OWFNamespace ReadNamespace(MemoryStream ms) {
            var signals = new List<OWFSignal>();
            var alarms = new List<OWFAlarm>();
            var events = new List<OWFEvent>();
            DateTime t0;
            TimeSpan dt;
            string id;
            using (var br = new BinaryReader(ms, Encoding.UTF8)) {
                var length = ReadU32(br);
                uint offset;
                using (var namespaceStream = new MemoryStream(ms.GetBuffer(), (int)ms.Position, (int)length, false)) {
                    t0 = ReadOWFTimestamp(br);
                    dt = new TimeSpan(checked((long)ReadU64(br)));
                    id = ReadOWFString(br);

                    var signalsLength = ReadU32(br);
                    offset = (uint)namespaceStream.Position;
                    using (var signalStream = new MemoryStream(ms.GetBuffer(), (int)ms.Position, (int)signalsLength, false)) {
                        while (offset < signalsLength) {
                            using (var sbr = new BinaryReader(signalStream, Encoding.UTF8)) {
                                signals.Add(ReadSignal(sbr));
                            }
                            // no seek...ReadSignal advances stream for us
                        }
                    }
                    namespaceStream.Seek(signalsLength, SeekOrigin.Current);

                    var eventsLength = ReadU32(br);
                    offset = (uint)namespaceStream.Position;
                    using (
                        var eventsStream = new MemoryStream(ms.GetBuffer(), (int)ms.Position, (int)eventsLength, false)) {
                        while (offset < eventsStream.Length) {
                            using (var ebr = new BinaryReader(eventsStream, Encoding.UTF8)) {
                                events.Add(ReadEvent(ebr));
                            }
                            // no seeking because ReadEvent moves the stream for us
                        }
                    }
                    namespaceStream.Seek(eventsLength, SeekOrigin.Current);

                    var alarmsLength = ReadU32(br);
                    offset = (uint)namespaceStream.Position;
                    using (
                        var alarmsStream = new MemoryStream(ms.GetBuffer(), (int)ms.Position, (int)alarmsLength, false)) {
                        while (offset < alarmsStream.Length) {
                            using (var abr = new BinaryReader(alarmsStream, Encoding.UTF8)) {
                                alarms.Add(ReadAlarm(abr));
                            }
                            // no seeking because ReadAlarm moves the stream for us
                        }
                    }
                    namespaceStream.Seek(alarmsLength, SeekOrigin.Current);
                }
                ms.Seek(length, SeekOrigin.Current);
                return new OWFNamespace(id, t0, dt, signals, events, alarms);
            }
        }

        public static OWFChannel ReadChannel(MemoryStream ms) {
            using (var br = new BinaryReader(ms, Encoding.UTF8)) {
                var namespaces = new List<OWFNamespace>();
                var length = ReadU32(br);
                var id = ReadOWFString(br);

                var offset = (uint)ms.Position;
                while (offset < length) {
                    uint namespaceLength = ReadU32(br);
                    using (
                        var namespaceStream = new MemoryStream(ms.GetBuffer(), (int)offset, (int)namespaceLength, false)) {
                        var ns = ReadNamespace(namespaceStream);
                        namespaces.Add(ns);
                    }
                    offset += namespaceLength;
                    ms.Seek(namespaceLength, SeekOrigin.Current);
                }

                return new OWFChannel(id, namespaces);
            }
        }

        public static UInt32 ReadU32(BinaryReader br) {
            var val = br.ReadUInt32();
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                val = BitConverter.ToUInt32(valueBytes, 0);
            }

            return val;
        }

        public static UInt64 ReadU64(BinaryReader br) {
            var val = br.ReadUInt64();
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                val = BitConverter.ToUInt32(valueBytes, 0);
            }

            return val;
        }

        public static Int64 ReadS64(BinaryReader br) {
            var val = br.ReadInt64();
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                val = BitConverter.ToUInt32(valueBytes, 0);
            }

            return val;
        }

        public static UInt32 ReadHeader(BinaryReader br) {
            byte[] magic = {0x4f, 0x57, 0x46, 0x31};
            var rawMagic = br.ReadBytes(4);
            if (rawMagic.SequenceEqual(magic) == false) {
                throw new Exception("Bad header magic number");
            }

            return ReadU32(br);
        }

        public static OWFPackage Convert(byte[] package) {
            checked {
                using (var mem = new MemoryStream(package, false)) {
                    uint length = 0;
                    using (var br = new BinaryReader(mem, Encoding.UTF8)) {
                        length = ReadHeader(br);
                    }

                    var offset = (uint)mem.Position;
                    var channels = new List<OWFChannel>();
                    while (offset < length) {
                        uint channelLength;
                        using (var br = new BinaryReader(mem, Encoding.UTF8)) {
                            channelLength = ReadU32(br);
                        }

                        using (
                            var channelStream = new MemoryStream(mem.GetBuffer(), (int)offset, (int)channelLength, false)) {
                            var channel = ReadChannel(channelStream);
                            channels.Add(channel);
                        }
                        offset += channelLength;
                    }

                    return new OWFPackage(channels);
                }
            }
        }
    }
}