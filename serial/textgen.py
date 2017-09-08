# import serial
import subprocess
import time
import sys

#Can be Downloaded from this Link
#https://pypi.python.org/pypi/pyserial
# socat -d -d pty,raw,echo=0 pty,raw,echo=0

def generate_text(device):
	messageNumber = 0
	while True:
		# function to execute the command in the shell
		print("Sending info to: " + str(device))
		text_generate_command = "echo \"TEST \""+str(messageNumber)+" > " + str(device)
		subprocess.call(text_generate_command, shell=True)
		time.sleep(5)
		print("INFO SENDED")
		messageNumber+=1


# dummy = serial.Serial('/dev/pts/18', 9600, timeout=3)

# while 1:
# 	value = dummy.readline()
# 	print(value)

if __name__ == '__main__':
	generate_text(sys.argv[1])