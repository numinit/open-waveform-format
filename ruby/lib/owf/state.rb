# OWF: the Open Waveform Format
# @file owf/state.rb
# @author Morgan Jones <morgan@medicalinformaticscorp.com>

require 'json'
require 'date'

# A class to generate an OWF.
# Not production-ready; useful for generating test OWF packets.
module OWF
  class State
    V1_MAGIC ||= 0x4f574631

    # Pushes a string.
    # @param str The string to push.
    def push_str str, null_pad=true
      raise ArgumentError, 'str must be a String' unless str.is_a? String

      # Pad the length
      offset = 4 - (str.bytesize % 4)
      offset = 0 if offset == 4 and !null_pad
      len = offset + str.bytesize
      raise RuntimeError, 'effective length is not a multiple of 4'.freeze unless len % 4 == 0

      @fmt << 'L>'.freeze << "a#{len}".freeze
      @res << len << str.freeze
      self
    end
    alias :push_str_strict :push_str

    # Pushes an 8-bit int.
    # @param u8 The 8-bit int to push
    def push_u8 u8
      raise ArgumentError, 'u8 must be an Integer' unless u8.is_a? Integer
      raise ArgumentError, 'u8 out of range' unless u8 >= 0x00 and u8 <= 0xff
      @fmt << 'C'.freeze
      @res << u8
      self
    end
    alias :push_u8_strict :push_u8

    # Pushes a 16-bit int.
    # @param u16 The 16-bit int to push
    def push_u16 u16
      raise ArgumentError, 'u16 must be an Integer' unless u16.is_a? Integer
      raise ArgumentError, 'u16 out of range' unless u16 >= 0x0000 and u16 <= 0xffff
      @fmt << 'S>'.freeze
      @res << u16
      self
    end
    alias :push_u16_strict :push_u16

    # Pushes a 32-bit int.
    # @param u32 The u32 to push.
    def push_u32 u32
      raise ArgumentError, 'u32 must be an Integer' unless u32.is_a? Integer
      raise ArgumentError, 'u32 out of range' unless u32 >= 0x00000000 and u32 <= 0xffffffff
      @fmt << 'L>'.freeze
      @res << u32
      self
    end
    alias :push_u32_strict :push_u32

    # Pushes a signed 64-bit int.
    # @param s64 The s64 to push.
    def push_s64 s64
      raise ArgumentError, 's64 must be an Integer' unless s64.is_a? Integer
      raise ArgumentError, 's64 out of range' unless s64 >= -0x8000000000000000 && s64 <= 0x7fffffffffffffff
      @fmt << 'q!>'.freeze
      @res << s64
      self
    end
    alias :push_s64_strict :push_s64

    # Pushes a 64-bit int.
    # @param u64 The u64 to push.
    def push_u64 u64
      raise ArgumentError, 'u64 must be an Integer' unless u64.is_a? Integer
      raise ArgumentError, 'u64 out of range' unless u64 >= 0x0000000000000000 and u64 <= 0xffffffffffffffff
      @fmt << 'Q>'.freeze
      @res << u64
      self
    end
    alias :push_u64_strict :push_u64

    # Pushes a 64-bit float.
    # @param f64 The f64 to push
    def push_f64 f64
      if f64.is_a? Float or f64.is_a? Integer
        @fmt << 'G'.freeze
        @res << f64.to_f
      elsif f64.is_a? String
        push_f64_strict f64
      end

      self
    end

    # Pushes a f64 that's encoded in a String.
    # This method performs a strict conversion based on the OWF JSON spec.
    # @param f64 A f64 encoded in a String.
    def push_f64_strict f64
      raise ArgumentError, 'f64 must be a String' unless f64.is_a? String
      if f64 == 'Infinity'.freeze
        f64 = Float::INFINITY
      elsif f64 == '-Infinity'.freeze
        f64 = -Float::INFINITY
      elsif f64 == 'NaN'.freeze
        f64 = Float::NAN
      else
        f64 = Float(f64)
      end

      @fmt << 'G'.freeze
      @res << f64

      self
    end

    # Pushes a 64-bit timestamp
    # @param t64 The t64 to push.
    def push_t64 t64
      if t64.is_a? Integer
        push_s64 t64
      elsif t64.is_a? String
        push_t64_strict t64
      else
        raise ArgumentError, 't64 must be an Integer or String'
      end

      self
    end

    # Pushes a 64-bit timestamp, encoded as a String.
    # This method performs a strict conversion based on the OWF JSON spec.
    def push_t64_strict t64
      raise ArgumentError, 't64 must be a String' unless t64.is_a? String
      t = DateTime.parse(t64).to_time
      val = t.tv_sec * 10_000_000 + t.tv_nsec / 100
      push_s64 val

      self
    end

    # Parses a JSON blob, converting it into an OWF.
    # @param blob The JSON blob to parse
    def parse_json blob
      @fmt, @res = [], []

      json = JSON.parse(blob, symbolize_names: true)
      json.default_proc = -> h, k {raise KeyError, "key `#{k}' not found"}

      push_u32_strict V1_MAGIC
      length_wrap {
        json[:channels].each do |channel|
          length_wrap {
            push_str_strict channel[:id]
            channel[:namespaces].each do |namespace|
              length_wrap {
                push_t64_strict namespace[:t0]
                push_u64_strict num_to_duration(namespace[:dt])
                push_str_strict namespace[:id]

                length_wrap {
                  namespace[:signals].each do |signal|
                    push_str_strict signal[:id]
                    push_str_strict signal[:units]

                    length_wrap {
                      signal[:data].each do |float|
                        push_f64_strict float
                      end
                    }
                  end
                }

                length_wrap {
                  namespace[:events].each do |event|
                    push_t64_strict event[:t0]
                    push_str_strict event[:message]
                  end
                }

                length_wrap {
                  namespace[:alarms].each do |alarm|
                    # Time and duration
                    push_t64_strict alarm[:t0]
                    push_u64_strict num_to_duration(alarm[:dt])

                    # Level, volume, and padding
                    push_u8_strict alarm[:level]
                    push_u8_strict alarm[:volume]
                    push_u16_strict 0

                    # Type and data
                    push_str_strict alarm[:type]
                    push_str_strict alarm[:message]
                  end
                }
              }
            end
          }
        end
      }

      pack
    end

    def self.json blob
      state = OWF::State.new
      state.parse_json blob
    end

    private

    def num_to_duration num
      (num * 10000).floor
    end

    # Wraps a block, prefixing it with that block's length.
    def length_wrap &block
      raise ArgumentError, 'must provide block' unless block_given?

      # Save the last item on the stack
      fmt, res = @fmt, @res

      # Make a new buffer
      @fmt, @res = [], []

      # Yield
      yield

      # Restore the state and just push the packed buffer as a string
      str = pack
      @fmt, @res = fmt, res
      push_str str, false

      self
    end

    # Packs the current state.
    def pack
      @res.pack @fmt.join
    end
  end
end
