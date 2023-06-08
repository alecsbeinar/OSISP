#!/bin/sh
gnome-terminal -- ./generate_unsorted_index 409600 file.bin
gnome-terminal -- ./print_index file.bin 0
gnome-terminal -- ./sort_index 40960 32 16 file.bin
gnome-terminal -- ./print_index sorted_data.bin 1
