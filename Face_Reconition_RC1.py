'''
  使用Haar Cascade进行人脸识别
  使用用LBP特征进行人脸匹配，可进行人脸注册、人脸检测与人脸识别
  UART1（Pin1）输出调试信息
  UART3（Pin4）输出识别结果，当识别成功后，返回“Find It”
  需使用SD卡，最大3支持2G，将main.py等文件放至SD卡根目录后上电
'''

import sensor, time, image ,display
import os, time
import pyb
from pyb import Pin
import math

red   = pyb.LED(1)
green = pyb.LED(2)
blue  = pyb.LED(3)
infrared = pyb.LED(4)
# 初始化UART，使用UART通道3，波特率9600,对应OpenMV与USB串口接线为p4接RX，p5接TX,GND接GND
usart3 = pyb.UART(3, 115200)
REGISTER_MODE = 0
IS_MATCH = 0


sensor.reset()
sensor.set_contrast(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.HQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)
# HQVGA和灰度对于人脸识别效果最好
sensor.skip_frames(10)
#lcd = display.SPIDisplay()

Path_Backup = {'path':'', 'id':0}
rootpath = "/reg_faces"
#DIST_THRESHOLD = 15000  # 差异度阈值，越大越宽容
DIST_THRESHOLD = 25000  # 差异度阈值
global mode


#定位长串口信息发送函数
def sending_data(a,x,y,z,c):
        sData = bytearray([a,x,y,z,c])
        usart3.write(sData)

#识别器
def detector(face_cascade, img):
    #objects = img.find_features(face_cascade, threshold=0.75, scale_factor=1.25)  # 人脸检测
    #objects = img.find_features(face_cascade, threshold=0.9, scale_factor=1.25)  # 人脸检测
    objects = img.find_features(face_cascade, threshold=0.9, scale_factor=1.35)
    if objects:
        blue.on()
        time.sleep(0.1)
        blue.off()
        width_old = 0
        height_old = 0
        index = 0
        for r in objects:  # 寻找最大的face
            if r[2] > width_old and r[3] > height_old:
                width_old = r[2]
                height_old = r[3]
                index += 1
        index -= 1
        #print("index:", index)
        img.draw_rectangle(objects[index])
        d0 = img.find_lbp((0, 0, img.width(), img.height()))
        res = matcher(d0)
        #若匹配上
        if res != 0:
            print(res)
            #取中心坐标
            center_x = objects[index][0] + 0.5*objects[index][2]
            center_y = objects[index][1] + 0.5*objects[index][3]
            #取整
            send_x = math.trunc(center_x)
            send_y = math.trunc(center_y)
            print(send_x,send_y)
            print("@%d,%d \r\n" % (send_x,send_y))

            #定位长串口发送
            sending_data(0xF3,send_x,send_y,0x01,0xF4)
            IS_MATCH = 1
            return 1
        else:
            #sending_data(0xF3,0,0,0xF4)
            #取中心坐标
            center_x = objects[index][0] + 0.5*objects[index][2]
            center_y = objects[index][1] + 0.5*objects[index][3]
            #取整
            send_x = math.trunc(center_x)
            send_y = math.trunc(center_y)
            print(send_x,send_y)
            print("@%d,%d \r\n" % (send_x,send_y))
            #定位长串口发送
            sending_data(0xF3,send_x,send_y,0x00,0xF4)
            print("No Face Detected")
    else:
        sending_data(0xF3,0,0,0,0xF4)
        print("No Face Detected")



#匹配器
def matcher(d0):  # 人脸识别
    dir_lists = os.listdir(rootpath)  # 路径下文件夹
    dir_num = len(dir_lists)          # 文件夹数量
    print("*" * 60)
    print("Total %d Folders -> %s"%(dir_num, str(dir_lists)))

    for i in range(0, dir_num):
        item_lists = os.listdir(rootpath+'/'+dir_lists[i])  # 路径下文件
        item_num = len(item_lists)                          # 文件数量
        print("The %d Folder[%s], Total %d Files -> %s" %(i+1, dir_lists[i], item_num, str(item_lists)))

        Path_Backup['path'] = rootpath+'/'+dir_lists[i]  # 马上记录当前路径
        Path_Backup['id'] = item_num                     # 马上记录当前文件数量

        for j in range(0, item_num):  # 文件依次对比
            print(">> Current File: " + item_lists[j])
            try:
                img = image.Image("/reg_faces/%s/%s" % (dir_lists[i], item_lists[j]), copy_to_fb=True)
                #lcd.write(img)  # 显示图像
            except Exception as e:
                print(e)
                break
            d1 = img.find_lbp((0, 0, img.width(), img.height()))  # 提取特征值
            dist = image.match_descriptor(d0, d1)                 # 计算差异度
            print(">> Difference Degree: " + str(dist))
            if dist < DIST_THRESHOLD:
                print(">> ** Find It! **")
                green.on()
                time.sleep(0.2)
                green.off()
                if IS_MATCH == 1:
                    green.on()
                    time.sleep(0.2)
                    green.off()
                return item_lists[j]
    print(">> ** No Match! **")
    return 0


#注册器
def register(face_cascade, img):
    global REGISTER_MODE
    #if detector(face_cascade, img) == 1:
     #   print(">> Existing without registration!")
      #  REGISTER_MODE = 0
       # return 0

    dir_lists = os.listdir(rootpath)  # 路径下文件夹
    dir_num = len(dir_lists)          # 文件夹数量
    new_dir = ("%s/%d") % (rootpath, int(dir_num)+1)
    os.mkdir(new_dir)                 # 创建文件夹
    cnt = 5  # 拍摄5次图片
    while cnt:
        img = sensor.snapshot()
        #objects = img.find_features(face_cascade, threshold=0.9, scale_factor=1.25)  # 人脸检测
        objects = img.find_features(face_cascade, threshold=0.9, scale_factor=1.35)
        #image.find_features(cascade, threshold=0.5, scale=1.5),thresholds越大，
        #匹配速度越快，错误率也会上升。scale可以缩放被匹配特征的大小。
        if objects:
            width_old = 0
            height_old = 0
            index = 0
            for r in objects:  # 寻找最大的face
                if r[2] > width_old and r[3] > height_old:
                    width_old = r[2]
                    height_old = r[3]
                    index += 1
            index -= 1
            #print("index:", index)

            item_lists = os.listdir(new_dir)  # 新路径下文件
            item_num = len(item_lists)        # 文件数量
            img.save("%s/%d.pgm" % (new_dir, item_num))  # 写入文件
            print(">> [%d]Regist OK!" % cnt)
            img.draw_rectangle(objects[index])
            red.on()
            time.sleep(0.1)
            red.off()
            cnt -= 1
            if cnt==0:
                green.on()
                time.sleep(1)
                green.off()
        REGISTER_MODE = 0

global data

def main():
    global REGISTER_MODE
    try:
        os.mkdir(rootpath)
    except:
        pass

    face_cascade = image.HaarCascade("frontalface", stages=25)
    print(face_cascade)
    clock = time.clock()
    img = None


    while (True):
        clock.tick()
        img = sensor.snapshot()
        #red.on()
        time.sleep(0.1)
        #red.off()
        mode=usart3.readchar()
        if mode == 0x05:
            REGISTER_MODE = 1
            print(REGISTER_MODE)
        else:
            REGISTER_MODE = 0
            print(REGISTER_MODE)

        if REGISTER_MODE == 1:
            print("REGISTER_MODE\r\n")
            register(face_cascade, img)
        else:
            res = detector(face_cascade, img)
            if res==1:
                print("Find It\r\n")

        #lcd.write(img)  # 显示图像
        # 打印FPS
        print(clock.fps())


main()
