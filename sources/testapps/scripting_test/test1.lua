local table = require "table"

t = { }
t[1] = 20

print('Hello World', 'test', 13, true, nil, t, z)
print('Host name: ' .. Host.Name())
print('Host version: ' .. Host.Version())
print();

jpegImporter = Host.CreatePluginInstance('fmt_jpeg.JpegImporter')
pngImporter  = Host.CreatePluginInstance('PngImporter')
invertPlugin  = Host.CreatePluginInstance('Invert')

jpegExporter = Host.CreatePluginInstance('JpegExporter')
--Host.CreatePluginInstance()

print(jpegImporter:Name(), jpegImporter:Version(), jpegImporter:Type())
print(jpegImporter:FileTypeDescription())
print(table.concat(jpegImporter:SupportedExtensions(), ' '))


image = jpegImporter:ImportImage('image1.jpg')
a = 1

print('Image', image, image:Width(), image:Height(), image:BitsPerPixel(), image:PixelFormat())

print(jpegExporter:Name(), jpegExporter:Version(), jpegExporter:Type())
print(jpegExporter:FileTypeDescription(), jpegExporter:SupportedExtensions())
print(table.concat(jpegExporter:SupportedPixelFormats(), ' '))


jpegExporter:ExportImage('test.jpg', image)

textileGenerator = Host.CreatePluginInstance('GenerateTextileTexture')

print('TextileGenerator', textileGenerator)
print('TextileGenerator:Name', textileGenerator:Name())

textile = textileGenerator:GenerateImage()
jpegExporter:ExportImage('test2.jpg', textile)

image2 = Image.Create(320, 240, 'RGB24')
print('Image2', image2, image2:Width(), image2:Height(), image2:BitsPerPixel(), image2:PixelFormat())

print( "InvertPlugin", invertPlugin:Type() )
print( "Can process in place:", invertPlugin:CanProcessInPlace() )
print( "Can support RGB24: ", invertPlugin:IsPixelFormatSupported('RGB24'))
print( "Output for RGB24: ", invertPlugin:GetOutputPixelFormat('RGB24'))

invertedTextile = invertPlugin:ProcessImage(textile)
jpegExporter:ExportImage('test2-inverted.jpg', invertedTextile)

invertPlugin:ProcessImageInPlace(textile)
jpegExporter:ExportImage('test2-inverted2.jpg', textile)

--grayscalePlugin  = Host.CreatePluginInstance('Grayscale')
--grayscalePlugin:ProcessImageInPlace(image)

thresholdPlugin  = Host.CreatePluginInstance('Threshold')
t = thresholdPlugin:GetProperty('threshold')
print('threshold property: ', t)

colorFilterPlugin  = Host.CreatePluginInstance('ColorFilter')
rangeRed = colorFilterPlugin:GetProperty('redRange')
fillColor = colorFilterPlugin:GetProperty('fillColor')

print('rangeRed: ', table.concat(rangeRed, ' '))
print('fillColor: ', table.concat(fillColor, ' '))

colorFilterPlugin:SetProperty('fillOutside', true)
colorFilterPlugin:SetProperty('fillOutside', false)
thresholdPlugin:SetProperty('threshold', 33)

t = thresholdPlugin:GetProperty('threshold')
print('threshold property: ', t)


colorFilterPlugin:SetProperty('redRange', {34, 178})
rangeRed = colorFilterPlugin:GetProperty('redRange')
print('rangeRed: ', table.concat(rangeRed, ' '))

colorFilterPlugin:SetProperty('fillColor', {40, 90, 170})
fillColor = colorFilterPlugin:GetProperty('fillColor')
print('fillColor: ', table.concat(fillColor, ' '))

filter = Host.CreatePluginInstance('GrayscaleGradientRecoloring')
filter:SetProperty('startColor', "400000" )
filter:SetProperty('endColor', "FFE0E0" )
filteredImage = filter:ProcessImage(textile)

subImage = textile:GetSubImage( 160, 120, 320, 240 )
filter:SetProperty('startColor', "000040" )
filter:SetProperty('endColor', "E0E0FF" )
filteredImage2 = filter:ProcessImage(subImage)

filteredImage:PutImage(filteredImage2, 160, 120)

--filteredImage.PutImage(filteredImage, filteredImage2, 240, 120 )
--filteredImage.PutImage(filteredImage, filteredImage2, true, 120 )


--[[
print( 'Source image size: ', filteredImage:Width( ), 'x', filteredImage:Height( ) )
subImage = filteredImage:GetSubImage( 160, 120, 320, 240 ):Clone( )
print( 'Sub image size: ', subImage:Width( ), 'x', subImage:Height( ) )
invertPlugin:ProcessImageInPlace(subImage)
--]]

jpegExporter:ExportImage('test3.jpg', filteredImage)

---[[
function Main()
	print('From Main 1, a=' .. a)
	a = a + 1
end
--]]
