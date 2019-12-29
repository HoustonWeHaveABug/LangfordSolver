if [ $# -ne 2 ]
then
	echo "Usage: $0 <inferior bound> <superior bound>"
	exit 1
fi
make -f langford_solver.make
echo "Pairs count planars only $1 $2"
echo 2 $1 $2 0 5 | ./langford_solver | grep "^[0-9]" | sed s/[ns]//g | sort -u | wc -l
exit 0
