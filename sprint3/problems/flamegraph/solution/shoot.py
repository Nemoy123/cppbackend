import argparse
import random
import shlex
import os
import subprocess, signal, time

RANDOM_LIMIT = 1000
SEED = 123456789
random.seed(SEED)

AMMUNITION = [
    'localhost:8080/api/v1/maps/map1',
    'localhost:8080/api/v1/maps'
]

SHOOT_COUNT = 100
COOLDOWN = 0.1
# 'perf record -o perf.data ' + 

def start_server():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', type=str)
    return parser.parse_args().server


def run(command, output=None):
    
    process = subprocess.Popen(shlex.split(command), stdout=output, shell=False)
    return process


def stop(process, wait=False):
    if process.poll() is None and wait:
        process.wait()
    
    process.terminate()


def shoot(ammo):
    hit = run('curl ' + ammo, output=subprocess.DEVNULL)
    time.sleep(COOLDOWN)
    stop(hit, wait=True)


def make_shots():
    for _ in range(SHOOT_COUNT):
        ammo_number = random.randrange(RANDOM_LIMIT) % len(AMMUNITION)
        shoot(AMMUNITION[ammo_number])
    print('Shooting complete')
    
# server = subprocess.Popen(start_server() + ' && perf record -o perf.data -p $!', shell=True)

server = run(start_server())
record = subprocess.Popen('perf record -o perf.data -g -p ' + str(server.pid), shell=True, preexec_fn=os.setsid)
# tmp = 'perf record -o perf.data -p '
# tmp = tmp + str(server.pid)
# record = subprocess.Popen(tmp, shell=True)

make_shots()

#record.send_signal(signal.CTRL_C_EVENT)   
os.killpg(os.getpgid(record.pid), signal.SIGTERM)
stop(record)
time.sleep(1)
#os.killpg(os.getpgid(server.pid), signal.SIGTERM)
stop(server)
#server.send_signal(signal.CTRL_C_EVENT) 
time.sleep(1)

#perf_script()
os.system('sudo perf script -i perf.data | sudo ./FlameGraph/stackcollapse-perf.pl | sudo ./FlameGraph/flamegraph.pl > graph.svg')
# first_mess = subprocess.Popen(shlex.split('perf script -f -i perf.data'), stdout=subprocess.PIPE)
# time.sleep(1)
# second_mess = subprocess.Popen('./FlameGraph/stackcollapse-perf.pl', stdin=first_mess.stdout, stdout=subprocess.PIPE)
# time.sleep(1)
# third_mess = subprocess.Popen(shlex.split('./FlameGraph/flamegraph.pl', '>', 'graph.svg'), stdin=second_mess.stdout)
# first_mess.stdout.close()
# second_mess.stdout.close()
#output = third_mess.comunicate()[0]
# os.system('sudo perf script -i perf.data | ./FlameGraph/stackcollapse-perf.pl | ./FlameGraph/flamegraph.pl > graph.svg')

# stop(perf)
#os.system('kill -INT $pid')
time.sleep(1)

print('Job done')



# server = run(start_server())
# 'perf record -o perf.data'
# run('perf record -o perf.data') perf record -p PID command
# sudo perf script | ./FlameGraph/stackcollapse-perf.pl | ./FlameGraph/flamegraph.pl > graph.svg
# server = run(start_server() + '&& perf record -o perf.data -p &!')
# record = run(start_perf_record(server))

# record = subprocess.Popen(shlex.split(tmp))
# perf = run('perf record -o perf.data ' + server)
# os.system('pid=$!')
# subprocess.run('pid=$!')