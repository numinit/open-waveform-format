using System;
using System.Text;

namespace OWF.DTO {
    public class OWFString : OWFObject {
        private UInt32 _stringSize, _paddingSize;
        private readonly string _str;

        public OWFString() {
            this._stringSize = this._paddingSize = UInt32.MaxValue;
            this._str = "";
        }

        public OWFString(string str) {
            this._stringSize = this._paddingSize = UInt32.MaxValue;
            this._str = str;
        }

        public string Value
        {
            get { return this._str; }
        }

        public UInt32 GetStringSizeInBytes() {
            if (this._stringSize == UInt32.MaxValue) {
                this._stringSize = (this.Value.Length == 0) ? 0 : ((uint)(Encoding.UTF8.GetByteCount(this.Value)) + 1);
            }
            return this._stringSize;
        }

        public UInt32 GetPaddingSizeInBytes() {
            if (this._paddingSize == UInt32.MaxValue) {
                var stringSize = this.GetStringSizeInBytes();
                var tmp = stringSize % 4;
                this._paddingSize = tmp == 0 ? 0 : 4 - tmp;
            }

            return this._paddingSize;
        }

        protected override UInt32 ComputeSizeInBytes() {
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