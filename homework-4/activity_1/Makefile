default: pipelined_ring_bcast


pipelined_ring_bcast: pipelined_ring_bcast.c
					docker run --rm -it -v $(dir $(abspath $(lastword $(MAKEFILE_LIST)))):/home/smpi --user $(id -u):$(id -g) henricasanova/ics632_smpi smpicc pipelined_ring_bcast.c -o pipelined_ring_bcast 

run50:
	 docker run --rm -it -v $(dir $(abspath $(lastword $(MAKEFILE_LIST)))):/home/smpi --user $(id -u):$(id -g) henricasanova/ics632_smpi smpirun --cfg=smpi/bcast:mpich -np 50 -platform ring_50.xml -hostfile hostfile_50.txt ./pipelined_ring_bcast pipelined_ring_bcast 1000000

run20:
	 docker run --rm -it -v $(dir $(abspath $(lastword $(MAKEFILE_LIST)))):/home/smpi --user $(id -u):$(id -g) henricasanova/ics632_smpi smpirun --cfg=smpi/bcast:mpich -np 20 -platform ring_20.xml -hostfile hostfile_20.txt ./pipelined_ring_bcast pipelined_ring_bcast 1000000

run5:
	 docker run --rm -it -v $(dir $(abspath $(lastword $(MAKEFILE_LIST)))):/home/smpi --user $(id -u):$(id -g) henricasanova/ics632_smpi smpirun --cfg=smpi/bcast:mpich -np 5 -platform ring_5.xml -hostfile hostfile_5.txt ./pipelined_ring_bcast pipelined_ring_bcast 1000000
