require 'json'

puts (JSON.pretty_generate({
	channels: 16.times.map {|i|
		{
			id: "CHANNEL_#{i}",
			namespaces: 16.times.map {|j|
				{
					id: "NS_#{i}_#{j}",
					t0: Time.now.utc.strftime('%FT%T.%7N'),
					dt: rand(0..1000000) + (rand(0..1000000) / 1.0e6),
					signals: 16.times.map {|k|
						{
							id: "SIG_#{i}_#{j}_#{k}",
							units: "fur_us",
							data: 16.times.map {|l| (rand(-0xffffffff..0xffffffff) / 0xffff.to_f).to_s}
						}
					},
					events: 16.times.map {|k|
						{
							t0: Time.now.utc.strftime('%FT%T.%7N'),
							message: "EVENT_#{i}_#{j}_#{k}"
						}
					},
					alarms: 16.times.map {|k|
						{
							t0: Time.now.utc.strftime('%FT%T.%7N'),
							dt: rand(0..1000000) + (rand(0..1000000) / 1.0e6),
							level: rand(0..255),
							volume: rand(0..255),
							type: "ALARM_#{i}_#{j}",
							message: "#{k}"
						}
					}
				}
			}
		}
	}
}, indent: '    '))
