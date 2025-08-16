rm -rf builds/*.log

./bpsim bimodal 6 gcc_trace.txt > builds/bmgc6.log
diff -iw builds/bmgc6.log val_outputs/bimodal_gcc_val0.txt

./bpsim bimodal 12 gcc_trace.txt > builds/bmgc12.log
diff -iw builds/bmgc12.log val_outputs/bimodal_gcc_val1.txt

./bpsim bimodal 4 jpeg_trace.txt > builds/bmjpeg4.log
diff -iw builds/bmjpeg4.log val_outputs/bimodal_jpeg_val2.txt

./bpsim bimodal 4 sample_trace.txt > builds/bmsam4.log
diff -iw builds/bmsam4.log sample_outputs/bimodal_sample0.txt


./bpsim gshare 4 2 sample_trace.txt > builds/gssam42.log
diff -iw builds/gssam42.log sample_outputs/gshare_sample0.txt

./bpsim gshare 9 3 gcc_trace.txt > builds/gsgc93.log
diff -iw builds/gsgc93.log val_outputs/gshare_gcc_val0.txt

./bpsim gshare 14 8 gcc_trace.txt > builds/gsgc148.log
diff -iw builds/gsgc148.log val_outputs/gshare_gcc_val1.txt

./bpsim gshare 11 5 jpeg_trace.txt > builds/gsjpeg115.log
diff -iw builds/gsjpeg115.log val_outputs/gshare_jpeg_val2.txt
