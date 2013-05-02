	.TITLE		GURIPA utility messages and codes
	.IDENT		'V01.00'
	.FACILITY	GURIPA,2/PREFIX=GUR_

	.SEVERITY WARNING
NOPRIV	<Insufficient privileges; WORLD and OPER are required>
NOMBAC	<No acces to control mailbox. Are you SYSTEM or have SYSPRV?>

	.SEVERITY ERROR
BADINT	<!%T: Invalid integer value for logical !AD>
TIMOUT	<Request to GURIPA timed out. Is it running?> 

	.SEVERITY FATAL
BADSYN	<Invalid syntax; internal error>
NOTRUN	<Mailbox does not exist. GURIPA doesn't seem to be running>

	.SEVERITY SUCCESS
STRTED  <GURIPA process started, pid=!XL>
EXITSN	<Exit request sent, GURIPA should stop immediately>

	.SEVERITY INFORMATIONAL
TSTAMP	<!%T: Timestamp>
STRTUP	<!%T: GURIPA starting up, intval=!SL, killint=!SL, msgint=!SL>
RUNNIN  <GURIPA is already running>
STATUS	<PID=!XL, interval=!SL, kill count=!SL, warn count=!SL>
EXIT	<!%T: Exit request received>
KILLED	<!%T: Process !AD (PID !XL) killed for inactivity>
