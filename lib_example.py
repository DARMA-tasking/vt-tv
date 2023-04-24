import my_ext
import block

circle1 = my_ext.Circle("Red")
circle1.what_color()
circle1.render() # exports test.png

b = block.Block(2,3,5.4,set({1,2,3,4}))
print(b.to_string())
print(b.attach_object_id(42))
# trying to attach an object id that is not an int actually throws a python TypeError
print(b.to_string())
b.detach_object_id(42)
print(b.to_string())