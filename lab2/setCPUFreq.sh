for i in $(seq 0 11);
do
    sudo cpufreq-set -d 3700000 -u 3700000 -c $i -r
done

sleep 5

for i in $(seq 0 11);
do
    sudo cpufreq-set -g performance -c $i -r
done
