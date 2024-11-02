function tdm --argument-names cmd --description "Shortcuts for Target Display Mode"
	switch "$cmd"
		case cmdf2 toggle tog
			ssh tdm.local "sudo cmdf2"
		case goodnight
			ssh tdm.local 'bash -c "sudo cmdf2; sleep 10; sudo shutdown -h now" </dev/null >/dev/null 2>/dev/null &'
		case fans
			ssh tdm.local /Applications/smcFanControl.app/Contents/Resources/smc -f
		case temps
			# https://apple.stackexchange.com/questions/54329/can-i-get-the-cpu-temperature-and-fan-speed-from-the-command-line-in-os-x
			ssh tdm.local /Applications/HardwareMonitor.app/Contents/MacOS/hwmonitor -c
		case hwinfo
			ssh tdm.local system_profiler SPHardwareDataType
		case tbinfo
			ssh tdm.local system_profiler SPThunderboltDataType
		case shutdown
			ssh tdm.local sudo /sbin/shutdown -h now
		case reboot
			ssh tdm.local sudo /sbin/shutdown -r now
		case ssh
			ssh tdm.local
		case screenshare
			#ssh -xfNMS "$HOME/.ssh/tdm-fwd" -L 5901:localhost:5900 tdm.local
			open vnc://tdm._rfb._tcp.local
		case \*
			echo "Usage: tdm {cmdf2|goodnight|fans|temps|hwinfo|tbinfo|shutdown|reboot|wake|ssh|screenshare}"
	end
end