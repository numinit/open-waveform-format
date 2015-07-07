using System;
using System.IO;
using System.Linq;
using System.Text;
using OWF.DTO;

namespace OWF.Serializers {
    /// <summary>
    /// A class representing state for packing OWF data.
    /// </summary>
    public class BinaryPacker {
        /// <summary>
        /// A BinaryWriter to write to.
        /// </summary>
        private BinaryWriter _bw;

        /// <summary>
        /// Writes an OWFString to the BinaryWriter.
        /// </summary>
        /// <param name="str">The string to write.</param>
        private void WriteOWFString(OWFString str) {
            checked {
                var fullSize = str.GetSizeInBytes() - sizeof(UInt32);

                // write length
                this.WriteLength(fullSize);

                // write string and null-terminator
                if (fullSize > 0) {
                    // Get the bytes, and write them, adding a null-terminator
                    var bytes = Encoding.UTF8.GetBytes(str.Value);
                    this._bw.Write(bytes);
                    this._bw.Write((byte)0);

                    // Subtract the length we just wrote
                    fullSize -= (UInt32)(bytes.Length + 1);
                }

                // write any padding
                while (fullSize > 0) {
                    this._bw.Write((byte)0);
                    fullSize--;
                }
            }
        }

        /// <summary>
        /// Writes an array of OWF doubles to the BinaryWriter.
        /// </summary>
        /// <param name="values">The values to write</param>
        private void WriteOWFDoubles(double[] values) {
            if (BitConverter.IsLittleEndian) {
                foreach (var valueBytes in values.Select(BitConverter.GetBytes)) {
                    Array.Reverse(valueBytes);
                    this._bw.Write(valueBytes);
                }
            }
            else {
                foreach (var v in values) {
                    this._bw.Write(v);
                }
            }
        }

        /// <summary>
        /// Writes an unsigned 64-bit integer to the BinaryWriter.
        /// </summary>
        /// <param name="val">The value to write</param>
        private void WriteU64(UInt64 val) {
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                this._bw.Write(valueBytes);
            }
            else {
                this._bw.Write(val);
            }
        }

        /// <summary>
        /// Writes a signed 64-bit integer to the BinaryWriter.
        /// </summary>
        /// <param name="val">The value to write</param>
        private void WriteS64(Int64 val) {
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                this._bw.Write(valueBytes);
            }
            else {
                this._bw.Write(val);
            }
        }

        /// <summary>
        /// Writes a length (32-bit multiple of 4) to the BinaryWriter.
        /// </summary>
        /// <param name="length">The length to write</param>
        private void WriteLength(UInt32 length) {
            if (length % sizeof(UInt32) != 0) {
                throw new PackError("length `{0}` was not a multiple of {1} bytes", length, sizeof(UInt32));
            }
            this.WriteU32(length);
        }

        /// <summary>
        /// Writes a 32-bit value to the BinaryWriter.
        /// </summary>
        /// <param name="val">The value to write</param>
        private void WriteU32(UInt32 val) {
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                this._bw.Write(valueBytes);
            }
            else {
                this._bw.Write(val);
            }
        }

        /// <summary>
        /// Writes the OWF header.
        /// </summary>
        /// <param name="length">The length</param>
        private void WriteHeader(UInt32 length) {
            this.WriteU32(OWFObject.Magic);
            this.WriteLength(length);
        }

        /// <summary>
        /// Writes an OWFChannel to the BinaryWriter.
        /// </summary>
        /// <param name="channel">The channel</param>
        private void WriteChannel(OWFChannel channel) {
            this.WriteLength(checked(channel.GetSizeInBytes() - sizeof(UInt32)));
            this.WriteOWFString(channel.Id);
            foreach (var ns in channel.Namespaces) {
                this.WriteNamespace(ns);
            }
        }

        /// <summary>
        /// Writes an OWFNamespace to the BinaryWriter.
        /// </summary>
        /// <param name="ns">The namespace</param>
        private void WriteNamespace(OWFNamespace ns) {
            this.WriteLength(checked(ns.GetSizeInBytes() - sizeof(UInt32)));
            this.WriteS64(ns.T0);
            this.WriteU64(ns.Dt);
            this.WriteOWFString(ns.Id);

            var signalsLength = ns.Signals.Aggregate(0U, (current, sig) => checked(current + sig.GetSizeInBytes()));
            var eventsLength = ns.Events.Aggregate(0U, (current, evt) => checked(current + evt.GetSizeInBytes()));
            var alarmsLength = ns.Alarms.Aggregate(0U, (current, alarm) => checked(current + alarm.GetSizeInBytes()));

            this.WriteLength(signalsLength);
            foreach (var sig in ns.Signals) {
                this.WriteSignal(ns, sig);
            }

            this.WriteLength(eventsLength);
            foreach (var evt in ns.Events) {
                this.WriteEvent(ns, evt);
            }

            this.WriteLength(alarmsLength);
            foreach (var alarm in ns.Alarms) {
                this.WriteAlarm(ns, alarm);
            }
        }

        /// <summary>
        /// Writes an OWFSignal to the BinaryWriter
        /// </summary>
        /// <param name="ns">The enclosing namespace</param>
        /// <param name="sig">The signal</param>
        private void WriteSignal(OWFNamespace ns, OWFSignal sig) {
            this.WriteOWFString(sig.Id);
            this.WriteOWFString(sig.Unit);

            var samplesLength = checked(sizeof(Double) * (UInt32)sig.Samples.Length);
            this.WriteLength(samplesLength);
            this.WriteOWFDoubles(sig.Samples);
        }

        /// <summary>
        /// Writes an OWFEvent to the BinaryWriter
        /// </summary>
        /// <param name="ns">The enclosing namespace</param>
        /// <param name="evt">The event</param>
        private void WriteEvent(OWFNamespace ns, OWFEvent evt) {
            if (!ns.Covers(evt.T0)) {
                throw new PackError("time interval for namespace `{0}` [{1}, {2}):{3} did not cover event at {4}",
                    ns.Id, ns.T0, ns.T0 + (Int64)ns.Dt, ns.Dt, evt.T0);
            }
            this.WriteS64(evt.T0);
            this.WriteOWFString(evt.Message);
        }

        /// <summary>
        /// Writes an OWFAlarm to the BinaryWriter
        /// </summary>
        /// <param name="ns">The enclosing namespace</param>
        /// <param name="alarm">The alarm</param>
        private void WriteAlarm(OWFNamespace ns, OWFAlarm alarm) {
            if (!ns.Covers(alarm.T0)) {
                throw new PackError("time interval for namespace `{0}` [{1}, {2}):{3} did not cover alarm at {4}",
                    ns.Id, ns.T0, ns.T0 + (Int64)ns.Dt, ns.Dt, alarm.T0);
            }
            this.WriteS64(alarm.T0);
            this.WriteU64((UInt64)alarm.TimeSpan.Ticks);
            this._bw.Write(alarm.Level);
            this._bw.Write(alarm.Volume);
            this._bw.Write((UInt16)0);
            this.WriteOWFString(alarm.Type);
            this.WriteOWFString(alarm.Message);
        }

        /// <summary>
        /// Converts an OWFPackage, writing its serialized form to the provided Stream.
        /// Throws a PackError if there's a problem.
        /// </summary>
        /// <param name="package">The package</param>
        /// <param name="stream">The stream</param>
        public void Convert(OWFPackage package, Stream stream) {
            lock (this) {
                using (this._bw = new BinaryWriter(stream, Encoding.UTF8)) {
                    this.WriteHeader(checked(package.GetSizeInBytes() - sizeof(UInt32)));
                    foreach (var channel in package.Channels) {
                        this.WriteChannel(channel);
                    }
                }
            }
        }

        /// <summary>
        /// Converts an OWFPackage, materializing it in memory and returning it as a byte array.
        /// Throws a PackError if there's a problem.
        /// </summary>
        /// <param name="package">The package</param>
        /// <returns>A byte array filled with OWF data</returns>
        public byte[] Convert(OWFPackage package) {
            using (var mem = new MemoryStream()) {
                this.Convert(package, mem);
                return mem.ToArray();
            }
        }

        /// <summary>
        /// Converts an OWFPackage, writing its serialized form to the provided Stream.
        /// Throws a PackError if there's a problem.
        /// </summary>
        /// <param name="package">The package</param>
        /// <param name="stream">The stream</param>
        public static void Pack(OWFPackage package, Stream stream) {
            new BinaryPacker().Convert(package, stream);
        }

        /// <summary>
        /// Converts an OWFPackage, materializing it in memory and returning it as a byte array.
        /// Throws a PackError if there's a problem.
        /// </summary>
        /// <param name="package">The package</param>
        /// <returns>A byte array filled with OWF data</returns>
        public static byte[] Pack(OWFPackage package) {
            return new BinaryPacker().Convert(package);
        }

        /// <summary>
        /// An error thrown if there is a problem packing the OWF.
        /// </summary>
        [Serializable]
        public class PackError : Exception {
            public PackError(String message, params Object[] args) : base(String.Format(message, args)) {}
            public PackError(String message, Exception cause, params Object[] args) : base(String.Format(message, args), cause) {}
        }
    }
}