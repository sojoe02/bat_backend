<?php
$output = explode("\n",`/usr/bin/perl /home/sojoe/Programs/bat_backend/zeroconf_init.plx`);
#$iarr = explode("\n",$output);
echo $output[1], "\n";
?>
