if [ $# -lt 2 ] || [ $# -gt 5 ]
then
	echo "Usage: $0 <inferior bound> <maximum superior bound> [ <settings> ] [ <sentinel> ] [ <dimensions> ]"
	exit 1
fi
make -f langford_solver.make
let IB=$1
shift
let MSB=$1
shift
let SB=$((6+($IB+2)/3*3))
while [ $SB -le $MSB ]
do
	echo 3 $IB $SB 0 $* | ./langford_solver
	if [ $SB -eq $MSB ]
	then
		exit 0
	fi
	let SB=$((SB+1))
	echo 3 $IB $SB 0 $* | ./langford_solver
	if [ $SB -eq $MSB ]
	then
		exit 0
	fi
	let SB=$((SB+1))
	echo 3 $IB $SB 0 $* | ./langford_solver
	let SB=$((SB+7))
done
exit 0
