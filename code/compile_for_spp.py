import os

Const_Image_Format = [".c", ".C"]


class FileFilt:
    fileList = [""]
    counter = 0

    def __init__(self):
        pass

    def FindFile(self, dirr, filtrate=1):
        global Const_Image_Format
        for s in os.listdir(dirr):
            newDir = os.path.join(dirr, s)
            if os.path.isfile(newDir):
                if filtrate:
                    if newDir and (os.path.splitext(newDir)[1] in Const_Image_Format):
                        self.fileList.append(s)
                        self.counter += 1
                else:
                    self.fileList.append(s)
                    self.counter += 1

b = FileFilt()
b.FindFile(dirr=os.getcwd())
cmd = "gcc"
for k in b.fileList:
    cmd += " "
    cmd += k
cmd += " -o spp"
print(cmd)
os.system(cmd)
print("Compile Success! execute program is built in /code/spp.exe\n")
os.system("pause")
