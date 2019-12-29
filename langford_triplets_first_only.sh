if [ $# -ne 1 ]
then
	echo "Usage: $0 <superior bound>"
	exit 1
fi
make -f langford_solver.make
let N1=9
let N2=10
let N3=11
while [ $N3 -le $1 ]
do
	echo "Triplets first only 1 $N1"
	echo 3 1 $N1 0 6 | ./langford_solver
	echo "Triplets first only 2 $N1"
	echo 3 2 $N1 0 6 | ./langford_solver
	echo "Triplets first only 1 $N2"
	echo 3 1 $N2 0 6 | ./langford_solver
	echo "Triplets first only 2 $N2"
	echo 3 2 $N2 0 6 | ./langford_solver
	echo "Triplets first only 1 $N3"
	echo 3 1 $N3 0 6 | ./langford_solver
	echo "Triplets first only 2 $N3"
	echo 3 2 $N3 0 6 | ./langford_solver
	let N1=$((N1+9))
	let N2=$((N2+9))
	let N3=$((N3+9))
done
