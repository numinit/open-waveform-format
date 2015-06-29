using System;
using System.Text;

namespace OWF.DTO {
    public class OWFString {
        private readonly string _str;

        public OWFString(string str) {
            this._str = str;
        }

        public string Value
        {
            get { return this._str; }
        }

        public UInt32 GetStringSizeInBytes() {
            return (this.Value.Length == 0) ? 0 : ((uint)(Encoding.UTF8.GetByteCount(this.Value)) + 1);
        }

        public UInt32 GetPaddingSizeInBytes() {
            var stringSize = this.GetStringSizeInBytes();
            var tmp = stringSize % 4;
            return tmp == 0 ? 0 : 4 - tmp;
        }

        public UInt32 GetSizeInBytes() {
            var stringSize = this.GetStringSizeInBytes();
            var paddingSize = this.GetPaddingSizeInBytes();
            return sizeof(UInt32) + stringSize + paddingSize;
        }

        public override bool Equals(object o) {
            if (o == null) {
                return false;
            }

            if (o is OWFString) {
                return this.Equals((OWFString)o);
            }
            if (o is string) {
                return this.Equals((string)o);
            }
            return false;
        }

        public bool Equals(OWFString other) {
            return this.Value.Equals(other.Value);
        }

        public bool Equals(string other) {
            return this.Value.Equals(other);
        }

        public override int GetHashCode() {
            return this.Value.GetHashCode();
        }
    }
}