set term png
set style data lines
set output 'gc.png'
set xlabel 'Nr of created objects'
set ylabel 'Mem Usage'
plot 'gc.dat' using 1:2 title "Mem"
