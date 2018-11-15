b = 1

---[[
function Main()
    a = Host.GetVariable( "a" )
    print( "thread, b:", b, ", a:", a )
    b = b + 1
    Host.SetVariable( "b", b )
end
--]]