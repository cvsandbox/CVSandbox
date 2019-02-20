# ip_stdimaging plug-in source files

# search path for source files
VPATH = ../../

#    CutImagePlugin.cpp CutImagePluginDescriptor.cpp \

# source files
SRC = ip_stdimaging.cpp \
	AddImagesPlugin.cpp AddImagesPluginDescriptor.cpp \
	BinaryDilatation3x3Plugin.cpp BinaryDilatation3x3PluginDescriptor.cpp \
	BinaryErosion3x3Plugin.cpp BinaryErosion3x3PluginDescriptor.cpp \
	BlurPlugin.cpp BlurPluginDescriptor.cpp \
	CannyEdgeDetectorPlugin.cpp CannyEdgeDetectorPluginDescriptor.cpp \
	BrightnessCorrectionPlugin.cpp BrightnessCorrectionPluginDescriptor.cpp \
	ColorChannelsFilterPlugin.cpp ColorChannelsFilterPluginDescriptor.cpp \
	ColorFilterPlugin.cpp ColorFilterPluginDescriptor.cpp \
	ContrastCorrectionPlugin.cpp ContrastCorrectionPluginDescriptor.cpp \
	ContrastStretchingPlugin.cpp ContrastStretchingPluginDescriptor.cpp \
	ConvolutionPlugin.cpp ConvolutionPluginDescriptor.cpp \
    CutImagePlugin.cpp CutImagePluginDescriptor.cpp \
	DiffImagesPlugin.cpp DiffImagesPluginDescriptor.cpp \
	DiffImagesThresholdedPlugin.cpp DiffImagesThresholdedPluginDescriptor.cpp \
	Dilatation3x3Plugin.cpp Dilatation3x3PluginDescriptor.cpp \
	DilatationPlugin.cpp DilatationPluginDescriptor.cpp \
	DistanceColorFilterPlugin.cpp DistanceColorFilterPluginDescriptor.cpp \
	EdgeDetectorPlugin.cpp EdgeDetectorPluginDescriptor.cpp \
	EmbedQuadrilateralPlugin.cpp EmbedQuadrilateralPluginDescriptor.cpp \
	ErodeEdgesPlugin.cpp ErodeEdgesPluginDescriptor.cpp \
	Erosion3x3Plugin.cpp Erosion3x3PluginDescriptor.cpp \
	ErosionPlugin.cpp ErosionPluginDescriptor.cpp \
	ExtractNRGBChannelPlugin.cpp ExtractNRGBChannelPluginDescriptor.cpp \
	ExtractQuadrilateralPlugin.cpp ExtractQuadrilateralPluginDescriptor.cpp \
	ExtractRGBChannelPlugin.cpp ExtractRGBChannelPluginDescriptor.cpp \
	GaussianBlurPlugin.cpp GaussianBlurPluginDescriptor.cpp \
	GaussianSharpenPlugin.cpp GaussianSharpenPluginDescriptor.cpp \
	GrayscalePlugin.cpp GrayscalePluginDescriptor.cpp \
	GrayscaleToRgbPlugin.cpp GrayscaleToRgbPluginDescriptor.cpp \
	GrayWorldNormalizationPlugin.cpp GrayWorldNormalizationPluginDescriptor.cpp \
	HistogramEqualizationPlugin.cpp HistogramEqualizationPluginDescriptor.cpp \
	HitAndMissPlugin.cpp HitAndMissPluginDescriptor.cpp \
	HslColorFilterPlugin.cpp HslColorFilterPluginDescriptor.cpp \
	HsvColorFilterPlugin.cpp HsvColorFilterPluginDescriptor.cpp \
	ImageStatisticsPlugin.cpp ImageStatisticsPluginDescriptor.cpp \
	IntersectImagesPlugin.cpp IntersectImagesPluginDescriptor.cpp \
	InvertPlugin.cpp InvertPluginDescriptor.cpp \
	LevelsLinearPlugin.cpp LevelsLinearPluginDescriptor.cpp \
	LevelsLinearGrayscalePlugin.cpp LevelsLinearGrayscalePluginDescriptor.cpp \
	MaskImagePlugin.cpp MaskImagePluginDescriptor.cpp \
	Mean3x3Plugin.cpp Mean3x3PluginDescriptor.cpp \
	MeanShiftPlugin.cpp MeanShiftPluginDescriptor.cpp \
	MergeImagesPlugin.cpp MergeImagesPluginDescriptor.cpp \
	MirrorPlugin.cpp MirrorPluginDescriptor.cpp \
	MorphologyOperatorPlugin.cpp MorphologyOperatorPluginDescriptor.cpp \
	ObjectsEdgesPlugin.cpp ObjectsEdgesPluginDescriptor.cpp \
	ObjectsOutlinePlugin.cpp ObjectsOutlinePluginDescriptor.cpp \
	ObjectsThickeningPlugin.cpp ObjectsThickeningPluginDescriptor.cpp \
	ObjectsThinningPlugin.cpp ObjectsThinningPluginDescriptor.cpp \
	OtsuThresholdPlugin.cpp OtsuThresholdPluginDescriptor.cpp \
	PixellatePlugin.cpp PixellatePluginDescriptor.cpp \
	ReplaceRGBChannelPlugin.cpp ReplaceRGBChannelPluginDescriptor.cpp \
	ResizeImagePlugin.cpp ResizeImagePluginDescriptor.cpp \
	RotateImagePlugin.cpp RotateImagePluginDescriptor.cpp \
	RotateImage90Plugin.cpp RotateImage90PluginDescriptor.cpp \
	RunLengthSmoothingPlugin.cpp RunLengthSmoothingPluginDescriptor.cpp \
	ShiftImagePlugin.cpp ShiftImagePluginDescriptor.cpp \
	SubtractImagesPlugin.cpp SubtractImagesPluginDescriptor.cpp \
	ThresholdPlugin.cpp ThresholdPluginDescriptor.cpp 

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_imaging \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS = -liplugin -lafx_imaging -lafx_types
