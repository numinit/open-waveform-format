# OWF: the Open Waveform Format
# @file json2owf.rb
# @author Morgan Jones <morgan@medicalinformaticscorp.com>

$: << File.join(File.dirname(__FILE__), 'lib')

require 'owf'

input, output = File.open(ARGV[0], 'r'), nil

begin
  json = OWF::State.json(input.read)
  output = File.open(ARGV[1], 'w')
  output.write json
ensure
  input.close
  output.close unless output.nil?
end

exit 0
