setHuePlugin  = Host.CreatePluginInstance( 'SetHue' )

---[[
function Main()
    image = Host.GetImage( )
    processedImage = setHuePlugin:ProcessImage( image )
    Host.SetImage( processedImage )

    videoSource = Host.GetVideoSource( )

    print( videoSource:Name( ), " :: ", videoSource:Type( ), " :: ", videoSource:Version( ) )
end
--]]