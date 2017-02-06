-- Creates the oneshot_info.h file from the relevant resource,
--  including the license header.
local text = io.read("*a") .. "\x00"
local ltx = [[The MIT License

Copyright (c) 2015 Mathew Velasquez (http://mathew.link/)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.]]

print("/* A derivative work of one of the INFORMATION text files in the https://github.com/mathewv/oneshot-legacy repository.")
print(" * To be specific, converted into an easy-to-embed C-hex-dump.")
print(" * Those files are licensed under the following license:")
print(" *")
for l in ltx:gmatch("[^\r\n]+") do
	print(" * " .. l)
end
print(" */")
print("")
local args = {...}
print("const unsigned char oneshot_text_" .. args[1] .. "[] = {")
while text:len() > 0 do
	local chunk = text:sub(1, 16)
	text = text:sub(17)
	io.write("\t")
	for i = 1, 16 do
		local b = chunk:byte(i)
		if b ~= nil then
			io.write(string.format("0x%02x, ", b))
		end
	end
	print()
end
print("};")
