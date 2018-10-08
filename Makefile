all:
	clear
	gcc sw_lib/sitewhere.c sw_lib/sitewhere_pb.c sw_lib/pb_encode.c sw_lib/pb_decode.c sw_lib/pb_common.c sw_lib/double_conversion.c sw_lib/sw_bridge.c sw_test.c -l paho-mqtt3c -o SW_Run
	
run:
	clear
	./SW_Run
	
