# OWF: the Open Waveform Format
# @file json2owf.rb
# @author Morgan Jones <morgan@medicalinformaticscorp.com>

$: << File.join(File.dirname(__FILE__), 'lib')

require 'owf'

in_file = ARGV[0]
if ARGV[1].nil?
  dir = File.dirname in_file
  ext = File.extname in_file
  base = File.basename in_file, ext
  out_file = File.join dir, "#{base.gsub(/json/, 'binary')}.owf"
else
  out_file = ARGV[1]
end
puts "#{in_file} => #{out_file}"
input, output = File.open(in_file, 'r'), nil

begin
  json = OWF::State.json(input.read)
  output = File.open(out_file, 'w')
  output.write json
ensure
  input.close
  output.close unless output.nil?
end

exit 0
