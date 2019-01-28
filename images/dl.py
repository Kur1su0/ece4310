from __future__ import print_function
import requests
import urllib
from bs4 import BeautifulSoup
path = 'ece4310/images/'
image_src = []
src = []
url = 'http://cecas.clemson.edu/~ahoover/ece431/images/'
r = requests.get(url)
if r.status_code is 200:
    print("successfully access the website")
else:
    print("error status code",r.status_code)
    exit (1)
data = r.text

soup = BeautifulSoup(data, 'html.parser')
#print(bs.prettify())
for a in soup.find_all('a',href=True):
    image_src.append(a['href'])

for i in range(len(image_src)):
    if i > 4:
        #src.append(str(image_src[i]))
        print("downloading:",str(image_src[i]) )
        with open(path + str(image_src[i]) ,'wb' ) as f:
	    f.write(  requests.get(url + str(image_src[i])).content )
        print ("success...")
        
print("Done!")
