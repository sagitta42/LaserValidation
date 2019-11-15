# X points dictionary = {x point : [label, color]}

# example:
# XP = {
# '2018-03-14': ['Pi day','r'],
# '2017-12-25': ['25 dec = 31 oct', 'b'],
# '2018-07-22': ['Pi approximation day', 'g']
# }

# store it in the file xpoints.py for specific use, not in xplot.py itself
# purple, brown, pink, gray
XP = {
# 2: ['lala','r']
       8892: ['calibrations','r'],
	   9711: [None, 'r'],
	   10301: [None, 'r'],
	   10727: [None, 'r'],
#	   12000: ['recabling PID 18 (rack 14 removed)', 'g'],
	   17057: [None, 'r'],
#	   17030: ['change front end boards', 'gray'],
	   18000: ['new reference channels for CNGS', 'b'],
#       19907: ['BTB threshold at 20', 'r'],
#       24277: ['trigger board change', 'r'],
      26580: ['new trigger', 'b'],
       # CRATE 1 recabling, profileID=20: first
       28734: ['first crate recabling', 'k'],
       # CRATE 2-3 recabling, profileID=21
       28892: ['other crate recablings', 'c'],
       # CRATE 4-6 recabling, profileID=22
       28930: [None, 'c'],
       # CRATE 7-9 recabling, profileID=23
       29011: [None, 'c'],
       # CRATE 11-12 recabling, profileID=24
       29059: [None, 'c'],
       # CRATE 13 recabling, profileID=25: last
       29091: ['last crate recabling','m'],
       29601: ['clock issue','pink'],
       # 30825: ['blackout', 'purple'],
       # 30949: [None, 'purple'],
       30425: ['laser  3.99 -> 6', 'g'],
       31949: ['laser controller broken', 'orange'],
       32202: ['new controller', 'r'],
       32422: ['open laser box, still old head', 'purple'],
       32457: ['readjusting laser', 'brown'],
       32480: [None, 'brown']
}
