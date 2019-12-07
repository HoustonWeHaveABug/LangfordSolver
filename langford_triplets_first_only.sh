make -f langford_solver.make
let N1=8
let N2=9
let N3=10
while [ 1 ]
do
	echo "Triplets $N1"
	echo 1 $N1 3 3 | ./langford_solver
	echo "Triplets $N2"
	echo 1 $N2 3 3 | ./langford_solver
	echo "Triplets $N3"
	echo 1 $N3 3 3 | ./langford_solver
	let N1=$((N1+9))
	let N2=$((N2+9))
	let N3=$((N3+9))
done
