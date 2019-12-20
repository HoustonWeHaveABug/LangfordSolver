if [ $# -ne 1 ]
then
	echo "Usage: $0 <N2 upper bound>"
	exit 1
fi
make -f langford_solver.make
let N1=4
let N2=5
while [ $N2 -le $1 ]
do
	echo "Pairs 1 $N1"
	echo 2 1 $N1 0 3 | ./langford_solver
	echo "Pairs 2 $N1"
	echo 2 2 $N1 0 3 | ./langford_solver
	echo "Pairs 1 $N2"
	echo 2 1 $N2 0 3 | ./langford_solver
	echo "Pairs 2 $N2"
	echo 2 2 $N2 0 3 | ./langford_solver
	let N1=$((N1+4))
	let N2=$((N2+4))
done
exit 0
