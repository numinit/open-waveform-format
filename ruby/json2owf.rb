# OWF: the Open Waveform Format
# @file json2owf.rb
# @author Morgan Jones <morgan@medicalinformaticscorp.com>

$: << File.join(File.dirname(__FILE__), 'lib')

require 'owf'

input, output = File.open(ARGV[0], 'r'), File.open(ARGV[1], 'w')

begin
  output.write OWF::State.json(input.read)
ensure
  input.close
  output.close
end

exit 0
