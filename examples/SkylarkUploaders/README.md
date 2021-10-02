# SkylarkUploaders



Skylark uploaders is a free C++ program that will create a website allowing to upload / download files.  It require Ultimate++ to be compiled. 

![Skylark Uploaders](https://i.imgur.com/hs2VVLF.png)

At first launch, the program will require you to create the master user :

![user creation](https://i.imgur.com/crcGPNX.png)

The default port of the website is 7979  (of course it can be changed via modifying the code):

![default port](https://i.imgur.com/TiwykeL.png)

the first page you will stand at is the authentication page:

![authentication](https://i.imgur.com/B6n80Ix.png)

When master user is connected. It can create new users. every users can delete or upload files. Sometime you would like to share file without create account for users to connect. To do it so then you can simply click the link button near the file you want to share to create a public link to this file :

![generate link](https://i.imgur.com/fDL1Nk8.png)

The generated link will be shown under the file row :



![](https://i.imgur.com/fl4KJf3.png)

This link is public and can be share. The max downloads value will ensure the link will no longer be available after a certain amount of downloading (here 4).



Known problem : 

-On tiny system like raspberry, huge file will make the program crash when someone try to download them (upload is working fine as long you got the necessary space) . 

-if file have non ascii characters in is name then the application wont be able to delete it. 

