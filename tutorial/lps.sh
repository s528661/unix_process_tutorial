#!/bin/sh -

for i in 1 2 3 4; do
        for j in 0 1 2 3 4 5 6 7 8 9; do
                ps w -C forktest
                sleep 1
        done
done
