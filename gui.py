from tkinter import *
from tkinter import ttk
from tkinter import filedialog

window=Tk() #start

window.title('AES')        # title
#window.iconbitmap('.ico')  # setting icon (restricted .ico files)


frame_1=Frame(window)
frame_1.pack()

'''     send GUI    '''
def send_submit():
    data_text=send_gui_input.get()
    print("Send input-->",data_text)

def file_show():
    file_path=filedialog.askopenfilename()
    print(file_path)


send_gui=LabelFrame(frame_1,text="Send")
send_gui.grid(row=0,column=0)

send_gui_prompts=Label(send_gui,text="Enter text")    #input prompts
send_gui_prompts.grid(row=0,column=0)

#text area
send_gui_input=Entry(send_gui)      #input area
send_gui_input.grid(row=1,column=0)

send_gui_txt_1=Label(send_gui,text="    ")
send_gui_txt_1.grid(row=1,column=1)

#file area
send_gui_file_1=Label(send_gui,text="The file mush not exceed 10KB")
send_gui_file_1.grid(row=0,column=2)

send_gui_button=Button(send_gui,text="File",command=file_show,height=2,width=16)
send_gui_button.grid(row=1,column=2)

send_gui_file_1=Label(send_gui,text="*File Size Out of Range*",fg="red",font=('Arial'))
send_gui_file_1.grid(row=2,column=2)

send_gui_txt_2=Label(send_gui,text="    ")
send_gui_txt_2.grid(row=1,column=3)

#submit area
send_gui_button=Button(send_gui,text="Submit",command=send_submit,height=3,width=16)
send_gui_button.grid(row=1,column=4)


for widget in send_gui.winfo_children():        #spacing
    widget.grid_configure(padx=10,pady=5)



'''     reception GUI   '''


'''     other   '''

window.geometry("600x400")  #window size
window.mainloop()   #end