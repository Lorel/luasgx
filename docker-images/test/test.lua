-- print( sgxdecrypt( sgxprocess( sgxencrypt('function(...) return \'not \' .. ... end'), sgxencrypt('very secret') ) ) )

function function_wrapper(func)
  local prefix = 'function(params) func = '
  local suffix = ' return func(table.unpack(cjson.decode(params))) end'

  return prefix .. func .. suffix  
end


function params_wrapper(...)
  return cjson.encode({...})
end


function sgx(func, ...)
  return sgxdecrypt(sgxprocess(sgxencrypt(function_wrapper(func)), sgxencrypt(params_wrapper(...))))
end


print(sgx('function(x) return x.a + x.b end', {a=1,b=2}))

Sgx = {}

function Sgx:function_wrapper (func)
  local prefix = 'function(params) func = '
  local suffix = ' return func(table.unpack(cjson.decode(params))) end'

  return prefix .. func .. suffix
end

function Sgx:params_wrapper (...)
  return cjson.encode({...})
end

function Sgx:exec (func, ...)
  return sgxdecrypt(sgxprocess(sgxencrypt(self:function_wrapper(func)), sgxencrypt(self:params_wrapper(...))))
end

print(Sgx:exec('function(x) return x.a + x.b end', {a=1,b=2}))

-- print(Sgx:exec('function(x,y,z) return x + y * z end', 1, 2, 3))

print(Sgx:exec('function(x,y,z) return ((x + y) * z) end', 1, 2, 3))

-- print( sgxdecrypt( sgxprocess( sgxencrypt('function(x) a = cjson.decode(x) return a.x * a.y end'), sgxencrypt( cjson.encode(a) ) ) ) )
print('titi')
a = {x=1,y=2}
print( sgxdecrypt( sgxprocess( sgxencrypt('function (x) a = cjson.decode(x) return (a.x * a.y) + a.x end'), sgxencrypt( cjson.encode(a) ) ) ) )

local code = [[
-- Decompiled using luadec 2.2 rev: 895d923 for Lua 5.3 from https://github.com/viruscamp/luadec
-- Command line: -- test/test.luac
function(x)
-- params : x
-- function num : 0 , upvalues : _ENV
return x
end
]]

-- print(code)
-- print(wrapper(code))
-- print(load(wrapper(code)))

local code2 = [[
function(x)
local params = table.unpack((cjson.decode)(x))
local func = (
-- Decompiled using luadec 2.2 rev: 895d923 for Lua 5.3 from https://github.com/viruscamp/luadec
-- Command line: -- test/test.luac
function(x)
-- params : x
-- function num : 0 , upvalues : _ENV
return x.x * x.y * 5
end
)
return func(params)
end
]]

local code3 = [[
-- gmqvw
function(x,y)
-- yo
return x * y
end
]]


print( sgxdecrypt( sgxprocess( sgxencrypt('function() return \'toto\' end'), sgxencrypt( cjson.encode({a}) ) ) ) )

-- print( sgxdecrypt( sgxprocess( sgxencrypt(wrapper(code)), sgxencrypt( cjson.encode({a}) ) ) ) )

-- print( sgxdecrypt( sgxprocess( sgxencrypt(code2), sgxencrypt( cjson.encode({1,2}) ) ) ) )

-- print( sgxdecrypt( sgxprocess( sgxencrypt('function(x) local params = table.unpack((cjson.decode)(x)) return cjson.encode({params}) end'), sgxencrypt( cjson.encode({2,1,2}) ) ) ) )

print(function_wrapper(code3))

print( sgxdecrypt( sgxprocess( sgxencrypt(function_wrapper(code3)), sgxencrypt( params_wrapper(1,5) ) ) ) )

print(sgx(code3, 2, 5))
