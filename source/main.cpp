#include "winapishenanigans.h"
#include "display.h"
#include "text.h"
#include "uiface.h"
#include "serialize.h"

bool running = true;
int majorVersion = 0;
int minorVersion = 2;
int patchVersion = 0;
int mainWidth = 800;
int mainHeight = 600;

HINSTANCE ghInstance = nullptr;
HWND ghWnd = nullptr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	ghInstance = hInstance;

	winapi_initialize();
	display_initialize();
	text_initialize();
	uiface_initialize();

	display_resize(mainWidth, mainHeight);
	uiface_resize(mainWidth, mainHeight);

	UIScreen* editorScreen = new UIScreen();
	uiface_set_screen(editorScreen);

	int padding = 16;
	int paddingSm = 8;
	int i = padding;

	int standardWidth = 256;
	int standardHeight = 32;
	int standardHSpacing = standardWidth + padding;
	int standardVSpacing = standardHeight + paddingSm;

	int sliderWidth = standardWidth;
	int sliderHeight = 16;
	int sliderVSpacing = sliderHeight + paddingSm;
	

	UILabel* toolLabel = new UILabel("-> Pencil <-", 16, i, standardWidth, standardHeight, 0, 0, 0);
	i+= standardVSpacing;

	UIButton* clearButton = new UIButton("Clear", 16, i, standardWidth, standardHeight, 127, 0, 0);
	i += standardVSpacing;

	UIButton* pencilButton = new UIButton("Pencil", 16, i, standardWidth, standardHeight, 15, 0, 192);
	i += standardVSpacing;

	UIButton* lineButton = new UIButton("Line", 16, i, standardWidth, standardHeight, 127, 0, 192);
	i += standardVSpacing;

	UIButton* eraserButton = new UIButton("Erase", 16, i, standardWidth, standardHeight, 192, 63, 127);
	i += standardVSpacing;

	UIButton* fillButton = new UIButton("Fill", 16, i, standardWidth, standardHeight, 255, 0, 0);
	i += standardVSpacing;

	UISlider* toleranceSlider = new UISlider(16, i, sliderWidth, sliderHeight);
	UILabel* toleranceDisplayLabel = new UILabel("", 16, i, standardWidth, sliderHeight, 0, 0, 0);
	i += sliderVSpacing;

	UIButton* eyedropperButton = new UIButton("Pick Color", 16, i, standardWidth, standardHeight, 255, 63, 0);
	i += standardVSpacing;
	
	UISlider* redSlider = new UISlider(16, i, sliderWidth, sliderHeight);
	i += sliderVSpacing;

	UISlider* greenSlider = new UISlider(16, i, sliderWidth, sliderHeight);
	i += sliderVSpacing;

	UISlider* blueSlider = new UISlider(16, i, sliderWidth, sliderHeight);
	i += sliderVSpacing;

	UIRect* colorDisplay = new UIRect(16, i, standardWidth, standardHeight, 255, 255, 255);
	UILabel* colorDisplayLabel = new UILabel("", 16, i, standardWidth, standardHeight, 0, 0, 0);
	i += standardVSpacing;

	int editorWidth = 360;
	int editorHeight = 360;
	int editorButtonWidth = (editorWidth - padding) / 2;
	UIEditBitmap* imageEdit = new UIEditBitmap(padding + standardHSpacing, padding, editorWidth, editorHeight, 16, 16);

	UIButton* saveButton = new UIButton("Save",
	padding + standardHSpacing, editorHeight + padding * 2,
	editorButtonWidth, standardHeight,
	127, 0, 0);

	UIButton* loadButton = new UIButton("Load",
	padding + standardHSpacing + editorButtonWidth + 16, editorHeight + padding * 2,
	editorButtonWidth, standardHeight,
	127, 0, 0);
	
	UIButton* export1DButton = new UIButton("Export Array",
	padding + standardHSpacing, editorHeight + padding * 2 + 40,
	editorButtonWidth, standardHeight,
	127, 0, 0);

	UIButton* export2DButton = new UIButton("Export 2D Array",
	padding + standardHSpacing + editorButtonWidth + 16, editorHeight + padding * 2 + 40,
	editorButtonWidth, standardHeight,
	127, 0, 0);

	UIButton* gridButton = new UIButton("Grid: Off",
	padding * 2 + standardHSpacing + editorWidth, padding,
	128, standardHeight,
	127, 0, 0);

	auto editorToolsFunc = [toolLabel, imageEdit, clearButton, pencilButton, lineButton, eraserButton, fillButton, eyedropperButton, gridButton] (UIButton* button) {
		if (button == clearButton) {
			if (MessageBoxA(NULL, "Are you sure? This action cannot be undone.", "Riddle me this...", MB_YESNO | MB_ICONASTERISK) == IDYES)
				imageEdit->clear();
		} else if (button == pencilButton) {
			imageEdit->setDrawOperation(OPERATION_PENCIL);
			toolLabel->setText("-> Pencil <-");
		} else if (button == lineButton) {
			imageEdit->setDrawOperation(OPERATION_LINE);
			toolLabel->setText("-> Line <-");
		} else if (button == eraserButton) {
			imageEdit->setDrawOperation(OPERATION_ERASER);
			toolLabel->setText("-> Eraser <-");
		} else if (button == fillButton) {
			imageEdit->setDrawOperation(OPERATION_FILLBUCKET);
			toolLabel->setText("-> Fill Bucket <-");
		} else if (button == eyedropperButton) {
			imageEdit->setDrawOperation(OPERATION_EYEDROPPER);
			toolLabel->setText("-> Color Picker <-");
		} else if (button == gridButton) {
			imageEdit->setGridMode((imageEdit->getGridMode() + 1) % 3);
			switch(imageEdit->getGridMode()) {
				case 0:
					gridButton->setText("Grid: Off");
					break;
				case 1:
					gridButton->setText("Grid: Lines");
					break;
				case 2:
					gridButton->setText("Grid: Points");
					break;
			}
		}
	};
	clearButton->setClickFunc(editorToolsFunc);
	pencilButton->setClickFunc(editorToolsFunc);
	lineButton->setClickFunc(editorToolsFunc);
	eraserButton->setClickFunc(editorToolsFunc);
	fillButton->setClickFunc(editorToolsFunc);
	eyedropperButton->setClickFunc(editorToolsFunc);
	gridButton->setClickFunc(editorToolsFunc);

	auto serializeFunc = [imageEdit, saveButton, loadButton, export1DButton, export2DButton] (UIButton* button) {
		if (button == saveButton) {
			serialize_save_image(imageEdit->getImageWidth(), imageEdit->getImageHeight(), imageEdit->getImageData());
		} else if (button == loadButton) {
			int width, height;
			unsigned char* data = imageEdit->getImageData();
			serialize_load_image(&width, &height, &data);
			if (data) {
				imageEdit->reload(width, height, data);
			}
		} else if (button == export1DButton) {
			serialize_export_array1d(imageEdit->getImageWidth(), imageEdit->getImageHeight(), imageEdit->getImageData());
		} else if (button == export2DButton) {
			serialize_export_array2d(imageEdit->getImageWidth(), imageEdit->getImageHeight(), imageEdit->getImageData());
		}
	};
	saveButton->setClickFunc(serializeFunc);
	loadButton->setClickFunc(serializeFunc);
	export1DButton->setClickFunc(serializeFunc);
	export2DButton->setClickFunc(serializeFunc);

	// widget tooltips

	toolLabel->setTooltip(						"Selected tool preview."
												);
	clearButton->setTooltip(					"Clear the entire canvas. Cannot be undone."
												);
	pencilButton->setTooltip(					"Draw a freehand line."
												);
	lineButton->setTooltip(						"Draw a straight line."
												);
	eraserButton->setTooltip(					"Erase things. I know, surprising."
												);
	fillButton->setTooltip(						"Fill all adjacent pixels of the same color."
												);
	eyedropperButton->setTooltip(				"Pick a color from the canvas."
												);
	toleranceSlider->setTooltip(				"The fill bucket color similarity tolerance from\n"
												"0-255, with low values being more picky, and high\n"
												"values being more lenient."
												);
	colorDisplay->setTooltip(					"Selected color preview."
												);
	saveButton->setTooltip(						"Save the image to a .led file for later use."
												);
	loadButton->setTooltip(						"Load an image from a .led file."
												);
	export1DButton->setTooltip(					"Export the image to a Java integer array\n"
												"initializer. Each pixel has 3 color components\n"
												"ranging 0-255, laid out in RGB order."
												);
	export2DButton->setTooltip(					"Export the image to a 2D Java integer array\n"
												"initializer. Each pixel has 3 color components\n"
												"ranging 0-255, laid out in RGB order. The 2D array\n"
												"is laid out in [row][col] fashion."
												);
	gridButton->setTooltip(						"Show a grid of lines, points, or nothing at all."
												);

	toleranceSlider->setMaxColor(255, 255, 255);
	toleranceSlider->setValue(8);
	toleranceDisplayLabel->setColor(0, 0, 0, 0);
	toleranceDisplayLabel->setFont(defaultSmFont);
	toleranceDisplayLabel->setTextColor(0, 0, 0);
	redSlider->setMaxColor(255, 0, 0);
	greenSlider->setMaxColor(0, 255, 0);
	blueSlider->setMaxColor(0, 0, 255);
	colorDisplayLabel->setColor(0, 0, 0, 0);
	colorDisplayLabel->setFont(defaultSmFont);
	
	editorScreen->addUIWidget(toolLabel);
	editorScreen->addUIWidget(clearButton);
	editorScreen->addUIWidget(pencilButton);
	editorScreen->addUIWidget(lineButton);
	editorScreen->addUIWidget(eraserButton);
	editorScreen->addUIWidget(fillButton);
	editorScreen->addUIWidget(toleranceSlider);
	editorScreen->addUIWidget(toleranceDisplayLabel);
	editorScreen->addUIWidget(eyedropperButton);
	editorScreen->addUIWidget(redSlider);
	editorScreen->addUIWidget(greenSlider);
	editorScreen->addUIWidget(blueSlider);
	editorScreen->addUIWidget(colorDisplay);
	editorScreen->addUIWidget(colorDisplayLabel);
	editorScreen->addUIWidget(imageEdit);
	editorScreen->addUIWidget(saveButton);
	editorScreen->addUIWidget(loadButton);
	editorScreen->addUIWidget(export1DButton);
	editorScreen->addUIWidget(export2DButton);
	editorScreen->addUIWidget(gridButton);

	winapi_show();

	while (winapi_run()) {
		imageEdit->setFillTolerance(toleranceSlider->getValue());

		unsigned char r;
		unsigned char g;
		unsigned char b;
		if (imageEdit->getColorChanged()) {
			imageEdit->getDrawColor(&r, &g, &b);
			redSlider->setValue(r);
			greenSlider->setValue(g);
			blueSlider->setValue(b);
		} else {
			r = redSlider->getValue();
			g = greenSlider->getValue();
			b = blueSlider->getValue();
			imageEdit->setDrawColor(r, g, b);
		}

		colorDisplay->setColor(r, g, b);

		char text[64];

		sprintf(text, "Fill Tolerance: %u", toleranceSlider->getValue());
		toleranceDisplayLabel->setText(text);

		unsigned char invertR, invertG, invertB;
		uiface_smart_color_invert(r, g, b, &invertR, &invertG, &invertB);
		sprintf(text, "R: %u G: %u B: %u", r, g, b);
		colorDisplayLabel->setText(text);
		colorDisplayLabel->setTextColor(invertR, invertG, invertB);

		uiface_update();
		display_update();
	}

	delete editorScreen;

	uiface_shutdown();
	text_shutdown();
	display_shutdown();
	winapi_shutdown();

	return 0;
}