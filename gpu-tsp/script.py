import subprocess

command_fmt = "./parallel -t {} -c {} -s 42"

for i in range(13):
    for j in range(15):
        command = command_fmt.format(2 ** j, i)
        subprocess.run(command, shell=True)
