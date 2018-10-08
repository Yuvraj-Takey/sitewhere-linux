all:
	clear
	gcc -c pb_common.c pb_common.h pb_decode.c pb_decode.h pb_encode.c pb_encode.h sitewhere.c sitewhere.h sw_bridge.c sw_bridge.h double_conversion.c double_conversion.h sitewhere_pb.c sitewhere_pb.h -l paho-mqtt3c
	ar -cvq SW_Linux.a pb_common.o pb_decode.o pb_encode.o sitewhere.o sw_bridge.o double_conversion.o sitewhere_pb.o
	
cross:
	arm-cortexa9-linux-gnueabihf-gcc -c pb_common.c pb_common.h pb_decode.c pb_decode.h pb_encode.c pb_encode.h sitewhere.c sitewhere.h sw_bridge.c sw_bridge.h double_conversion.c double_conversion.h sitewhere_pb.c sitewhere_pb.h
	ar -cvq SW_Linux.a pb_common.o pb_decode.o pb_encode.o sitewhere.o sw_bridge.o double_conversion.o sitewhere_pb.o
	
