using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using OWF.DTO;

namespace OWF.Serializers {
    public class BinaryUnpacker {
        private BinaryReader _br;
        private UInt32 _segmentLength;

        /// <summary>
        /// The maximum size we're willing to allocate.
        /// </summary>
        private readonly UInt32 _maxAlloc;

        public BinaryUnpacker() : this(0x100000) {
        }

        public BinaryUnpacker(UInt32 maxAlloc) {
            this._maxAlloc = maxAlloc;
        }

        private delegate T ReaderDelegate<out T>();

        private OWFChannel ReadOWFChannel() {
            var id = this.Unwrap(this.ReadOWFString);
            return new OWFChannel(id, this.UnwrapNestedMulti(this.ReadOWFNamespace));
        }

        private OWFNamespace ReadOWFNamespace() {
            var t0 = this.ReadS64();
            var dt = this.ReadU64();
            var id = this.Unwrap(this.ReadOWFString);
            var signals = this.Unwrap(this.ReadOWFSignals);
            var events = this.Unwrap(this.ReadOWFEvents);
            var alarms = this.Unwrap(this.ReadOWFAlarms);
            return new OWFNamespace(id, t0, dt, signals, events, alarms);
        }

        private OWFSignal ReadOWFSignal() {
            var id = this.Unwrap(this.ReadOWFString);
            var unit = this.Unwrap(this.ReadOWFString);
            var rawSamples = this.Unwrap(this.ReadSegment);
            if (rawSamples.Length % sizeof(double) != 0) {
                throw new UnpackError("length of sample array is not {0}-byte aligned (got {1} bytes)", sizeof(double), rawSamples.Length);
            }

            var samples = new double[rawSamples.Length / sizeof(double)];
            if (BitConverter.IsLittleEndian) {
                for (var i = 0; i < rawSamples.Length; i += sizeof(double)) {
                    Array.Reverse(rawSamples, i, sizeof(double));
                    samples[i / sizeof(double)] = BitConverter.ToDouble(rawSamples, i);
                }
            }
            else {
                for (var i = 0; i < rawSamples.Length; i += sizeof(double)) {
                    samples[i / sizeof(double)] = BitConverter.ToDouble(rawSamples, i);
                }
            }
            return new OWFSignal(id, unit, samples);
        }

        private List<OWFSignal> ReadOWFSignals() {
            return this.UnwrapMulti(this.ReadOWFSignal);
        }

        private OWFEvent ReadOWFEvent() {
            var t0 = this.ReadS64();
            var message = this.Unwrap(this.ReadOWFString);
            return new OWFEvent(message, t0);
        }

        private List<OWFEvent> ReadOWFEvents() {
            return this.UnwrapMulti(this.ReadOWFEvent);
        }

        private OWFAlarm ReadOWFAlarm() {
            var t0 = this.ReadS64();
            var dt = this.ReadU64();
            var levelVolume = this.ReadU32();
            var level = (byte)((levelVolume & 0xff000000) >> 24);
            var volume = (byte)((levelVolume & 0x00ff0000) >> 16);
            var type = this.Unwrap(this.ReadOWFString);
            var message = this.Unwrap(this.ReadOWFString);
            return new OWFAlarm(type, message, t0, dt, level, volume);
        }

        private List<OWFAlarm> ReadOWFAlarms() {
            return this.UnwrapMulti(this.ReadOWFAlarm);
        }

        private UInt32 ReadLength() {
            var val = this.ReadU32();
            if (val % sizeof(UInt32) != 0) {
                throw new UnpackError("length was not {0}-byte aligned (got {1} bytes)", sizeof(UInt32), val);
            }
            return val;
        }

        private UInt32 ReadU32() {
            var val = this._br.ReadUInt32();
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                val = BitConverter.ToUInt32(valueBytes, 0);
            }

            this.MarkRead(sizeof(UInt32));
            return val;
        }

        private UInt64 ReadU64() {
            var val = this._br.ReadUInt64();
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                val = BitConverter.ToUInt64(valueBytes, 0);
            }

            this.MarkRead(sizeof(UInt64));
            return val;
        }

        private Int64 ReadS64() {
            var val = this._br.ReadInt64();
            if (BitConverter.IsLittleEndian) {
                var valueBytes = BitConverter.GetBytes(val);
                Array.Reverse(valueBytes);
                val = BitConverter.ToInt64(valueBytes, 0);
            }

            this.MarkRead(sizeof(Int64));
            return val;
        }

        private byte[] ReadSegment() {
            if (this._segmentLength > this._maxAlloc) {
                throw new UnpackError("allocated size was greater than max ({0} > {1})", this._segmentLength, this._maxAlloc);
            }
            else if (this._segmentLength == 0) {
                return new byte[] {};
            }
            else {
                // Read the rest of the segment, and mark the whole thing as read
                byte[] ret = this._br.ReadBytes(checked((Int32)this._segmentLength));
                this.MarkRead(this._segmentLength);
                return ret;
            }
        }

        public OWFString ReadOWFString() {
            OWFString ret;
            if (this._segmentLength > 0) {
                var rawString = this.ReadSegment();

                // Make sure that the string is null-terminated
                if (rawString[rawString.Length - 1] != 0) {
                    throw new UnpackError("string was not NULL-terminated");
                }

                // Get the actual length of this string (cutting out null padding)
                int i = 0;
                while (rawString[i] != 0) {
                    i++;
                }

                // Get a UTF-8 view of the part before the null bytes
                ret = new OWFString(Encoding.UTF8.GetString(rawString, 0, i));
            }
            else {
                ret = new OWFString();
            }

            return ret;
        }

        private T Unwrap<T>(ReaderDelegate<T> d) {
            UInt32 oldLength = this._segmentLength;
            try {
                // Unwrap the value
                UInt32 length = 0;
                var val = this.UnwrapValue(d, ref(length));

                // Restore the length, and mark that many bytes as read
                this._segmentLength = oldLength;
                this.MarkRead(length);

                // Return the unwrapped value
                return val;
            }
            catch (UnpackError) {
                this._segmentLength = oldLength;
                throw;
            }
        }

        private List<T> UnwrapNestedMulti<T>(ReaderDelegate<T> d) {
            var ret = new List<T>();
            while (this._segmentLength > 0) {
                ret.Add(this.Unwrap(d));
            }
            return ret;
        }

        private List<T> UnwrapMulti<T>(ReaderDelegate<T> d) {
            var ret = new List<T>();
            while (this._segmentLength > 0) {
                ret.Add(d());
            }
            return ret;
        }

        private T UnwrapValue<T>(ReaderDelegate<T> d, ref UInt32 outputLength) {
            // Read the length
            var length = this.ReadLength();

            // Store the old length, and push the new length
            this._segmentLength = length;

            // Call the delegate
            var ret = d();

            // Ensure that we have no trailing bytes
            if (this._segmentLength > 0) {
                throw new UnpackError("trailing data when reading segment: {0} bytes", this._segmentLength);
            }

            // Store how many bytes we read, and return the unwrapped value
            outputLength = length + sizeof(UInt32);
            return ret;
        }

        private List<OWFChannel> UnwrapTop() {
            return this.UnwrapNestedMulti(this.ReadOWFChannel);
        }

        public OWFPackage Convert(Stream str) {
            lock (this) {
                using (this._br = new BinaryReader(str, Encoding.UTF8)) {
                    // Read the header
                    UInt32 length = sizeof(UInt32);
                    this._segmentLength = length;
                    var magic = this.ReadU32();
                    if (magic != OWFObject.Magic) {
                        throw new UnpackError("invalid magic header: {0:X8}", magic);
                    }

                    // Deserialize the OWFPackage
                    this._segmentLength = length;
                    return new OWFPackage(this.UnwrapValue(this.UnwrapTop, ref(length)));
                }
            }
        }

        public OWFPackage Convert(byte[] package) {
            using (var mem = new MemoryStream(package, false)) {
                return this.Convert(mem);
            }
        }

        private void MarkRead(UInt32 bytes) {
            this._segmentLength = Sub32(this._segmentLength, bytes);
        }

        private static UInt32 Add32(UInt32 a, UInt32 b) {
            try {
                return checked(a + b);
            }
            catch (System.OverflowException e) {
                throw new UnpackError("unsigned 32-bit addition overflow ({0} + {1})", e, a, b);
            }
        }

        private static UInt32 Sub32(UInt32 a, UInt32 b) {
            if (a < b) {
                throw new UnpackError("unsigned 32-bit subtraction underflow ({0} - {1})", a, b);
            }
            return checked(a - b);
        }

        private static UInt32 Mul32(UInt32 a, UInt32 b) {
            try {
                return checked(a * b);
            }
            catch (System.OverflowException e) {
                throw new UnpackError("unsigned 32-bit multiplication overflow ({0} * {1})", e, a, b);
            }
        }

        [Serializable]
        public class UnpackError : Exception {
            public UnpackError(String message, params Object[] args) : base(String.Format(message, args)) {}
            public UnpackError(String message, Exception cause, params Object[] args) : base(String.Format(message, args), cause) {}
        }
    }
}