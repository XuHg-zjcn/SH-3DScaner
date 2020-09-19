# SH-3DScaner 3D扫描仪
## DIY Laser 3D Scaner, request list:
name               |  para  |description
-------------------|:------:|-----------
Linear laser diode |     5mW|3V Red-650nm D=9mm
5V step motor      |28BYJ-48|4096 pulse a circle
MCU                | STM32F1|request USB support
Android smartphone |Honor 8C|Camera and process image
special phone case |3D Print|fixed step motor and PCB on phone
linker             |3D Print|for laser diode and step motor
microUSB plug      |optional|USB OTG communication, power support
PCB and components |optional|power EMI filter, step motor driver, LD driver
9-DoF IMU          |optional|my phone have no Gyroscope

## some problems
there is without STM32 Code and 3D print model, I'll make these a few days later.  
I'm using Honor 8C phone, the 3D print phone case maybe can't fit your phone  
I'm can not draw PCB, so I use buyed ready-made boards  
暂时没有STM32代码和3D打印模型，几天后我再做。我使用的是荣耀8C手机，手机壳的3D打印模型可能不适合你。
我不会画PCB，暂时用网上买到的现成的板和模块

## the process pseudo-code: 流程伪代码(Python)
``````````````````````````````````````````````````````````python
while(!Get_all_line_in_camera_scope):
    if(i%num == 0):             #turn-off LD to calibration img0, every some frames
        img0 = cam.get_image()
        continue
    LD.light_pulse()            #duty 10-30%, make higher SNR, backgroud darkness
    img1 = cam.get_image()      #sync shutter during pulse, shutter time 1/300-1/100s
    step_motor.run_a_step()
    img2 = img1 - img0          #use IMU and Optflow to align image
    img0 = filter(img1)         #update img0, after filter
    line = get_line(img2)       #track pixel
    distance=calc(line)         #calc distance between object and camera
    i+=1
reconstruction_3D_model()
``````````````````````````````````````````````````````````

the LD duty can't too little, because image sensor is rolling shutter. if flash time less, the valid image window will narrow.  
激光二极管的占空比不能太小，因为图像传感器是卷帘快门。如果闪光时间太短，有效的图像窗口会变窄。  
