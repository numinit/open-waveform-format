# OWF: the Open Waveform Format
# @file owf/state.rb
# @author Morgan Jones <morgan@medicalinformaticscorp.com>

module OWF
  module Version
    # The version of OWF
    VERSION     ||= '0.1'.freeze
    SHORT_NAME  ||= OWF.to_s.freeze
    SHORT_IDENT ||= "#{SHORT_NAME} v#{VERSION}".freeze
    NAME        ||= SHORT_NAME
    IDENT       ||= SHORT_IDENT
  end

  include Version
end
