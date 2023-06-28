import sys, time

T = 2                       # sampling period of temperature sensor, in seconds
starttime = time.time()     # the start time, to calculate time offsets

with open("/sys/class/thermal/thermal_zone0/temp", "r") as temperaturefile:
    try: 
        while True:
            temperaturefile.seek(0)                             # read from start of file
            temperature = float(temperaturefile.read())/1000.0  # temperature is 5-digits, convert to float and divide by 1000 to get temperature in Celsius
            print(f"CPU temperature:\t{temperature:.3f} ºC")    # print it out
            time.sleep(T - (time.time() - starttime)%T)         # wait so that samples are printed every T seconds
    except KeyboardInterrupt:                                   # if the process is killed,
        print("\nclosing files")
        temperaturefile.close()                                 # close the temperature source file
        sys.exit()
