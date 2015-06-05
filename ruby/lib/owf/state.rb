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
    def push_str str
      raise ArgumentError, 'str must be a String' unless str.is_a? String
      len = (str.bytesize % 4 == 0 ? str.bytesize : 4 * (str.bytesize / 4 + 1))
      @fmt << 'L>'.freeze << "a#{len}".freeze
      @res << len << str.freeze
      self
    end
    alias :push_str_strict :push_str

    # Pushes a 32-bit int.
    # @param u32 The u32 to push.
    def push_u32 u32
      raise ArgumentError, 'u32 must be an Integer' unless u32.is_a? Integer
      @fmt << 'L>'.freeze
      @res << u32
      self
    end
    alias :push_u32_strict :push_u32

    # Pushes a 64-bit int.
    # @param u64 The u64 to push.
    def push_u64 u64
      raise ArgumentError, 'u64 must be an Integer' unless u64.is_a? Integer
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
        push_u64 t64
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
      push_u64 val

      self
    end

    # Parses a JSON blob, converting it into an OWF.
    # @param blob The JSON blob to parse
    def parse_json blob
      @fmt, @res = [], []

      json = JSON.parse(blob, symbolize_names: true)

      push_u32_strict V1_MAGIC
      length_wrap {
        json[:channels].each do |channel|
          length_wrap {
            push_str_strict channel[:id]
            channel[:namespaces].each do |namespace|
              length_wrap {
                push_t64_strict namespace[:t0]
                push_u64_strict namespace[:dt]
                push_str_strict namespace[:id]

                length_wrap {
                  namespace[:signals].each do |signal|
                    length_wrap {
                      push_str_strict signal[:id]
                      push_str_strict signal[:units]

                      length_wrap {
                        signal[:data].each do |float|
                          push_f64_strict float
                        end
                      }
                    }
                  end
                }

                length_wrap {
                  namespace[:events].each do |event|
                    push_t64_strict event[:time]
                    push_str_strict event[:data]
                  end
                }

                length_wrap {
                  namespace[:alarms].each do |alarm|
                    push_t64_strict alarm[:time]
                    push_str_strict alarm[:data]
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
      push_str str

      self
    end

    # Packs the current state.
    def pack
      @res.pack @fmt.join
    end
  end
end
