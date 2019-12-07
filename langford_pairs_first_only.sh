make -f langford_solver.make
let N1=3
let N2=4
while [ 1 ]
do
	echo "Pairs $N1"
	echo 1 $N1 2 3 | ./langford_solver
	echo "Pairs $N2"
	echo 1 $N2 2 3 | ./langford_solver
	let N1=$((N1+4))
	let N2=$((N2+4))
done
