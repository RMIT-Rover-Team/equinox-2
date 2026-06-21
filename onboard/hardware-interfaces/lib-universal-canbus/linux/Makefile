testMaster:
	@g++ -o TestLinuxMaster TestLinuxMaster.cpp ../libuniversalcan/RoverCanMaster.cpp ../libuniversalcan/SocketCanWrapper.cpp ../libuniversalcan/CommandUtils.cpp -I. -I../libuniversalcan/ 

testSlave:
	@g++ -o TestLinuxSlave TestLinuxSlave.cpp ../libuniversalcan/RoverCanSlave.cpp ../libuniversalcan/SocketCanWrapper.cpp ../libuniversalcan/CommandUtils.cpp -I. -I../libuniversalcan/ 

UniCanMon:
	@g++ -o UniCanMon UniCanMon.cpp ../libuniversalcan/RoverCanMaster.cpp ../libuniversalcan/SocketCanWrapper.cpp ../libuniversalcan/CommandUtils.cpp -I. -I../libuniversalcan/ 

testBroadCastReceive:
	@g++ -o TestBroadRecv TestBroadcastReceive.cpp ../libuniversalcan/RoverCanMaster.cpp ../libuniversalcan/SocketCanWrapper.cpp ../libuniversalcan/CommandUtils.cpp -I. -I../libuniversalcan/ 

all: testMaster testSlave UniCanMon testBroadCastReceive

clean:
	@rm -f TestLinuxMaster TestLinuxSlave UniCanMon