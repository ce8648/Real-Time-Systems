for i in $(seq 0 11);
do
    sudo cpufreq-set -u 4600000 -c $i -r
done

sleep 5

for i in $(seq 0 11);
do
    sudo cpufreq-set -g powersave -c $i -r
done
