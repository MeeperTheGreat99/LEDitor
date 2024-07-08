#include "winapishenanigans.h"
#include "display.h"
#include "text.h"
#include "uiface.h"
#include "serialize.h"

bool running = true;
int majorVersion = 0;
int minorVersion = 1;
int patchVersion = 0;

HINSTANCE ghInstance = nullptr;
HWND ghWnd = nullptr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	ghInstance = hInstance;

	winapi_initialize();
	display_initialize();
	text_initialize();
	uiface_initialize();

	display_resize(640, 480);
	uiface_resize(640, 480);

	UIScreen* editorScreen = new UIScreen();
	uiface_set_screen(editorScreen);

	int i = 16;
	int buttonSpacing = 40;
	int sliderSpacing = 24;

	UILabel* toolLabel = new UILabel("-> Pencil <-", 16, i, 128, 32, 0, 0, 0);
	i+= buttonSpacing;

	UIButton* clearButton = new UIButton("Clear", 16, i, 128, 32, 127, 0, 0);
	i += buttonSpacing;

	UIButton* pencilButton = new UIButton("Pencil", 16, i, 128, 32, 15, 0, 192);
	i += buttonSpacing;

	UIButton* lineButton = new UIButton("Line", 16, i, 128, 32, 127, 0, 192);
	i += buttonSpacing;

	UIButton* eraserButton = new UIButton("Erase", 16, i, 128, 32, 192, 63, 127);
	i += buttonSpacing;
	
	UISlider* redSlider = new UISlider(16, i, 128, 16);
	i += sliderSpacing;

	UISlider* greenSlider = new UISlider(16, i, 128, 16);
	i += sliderSpacing;

	UISlider* blueSlider = new UISlider(16, i, 128, 16);
	i += sliderSpacing;

	UIRect* colorDisplay = new UIRect(16, i, 128, 32, 255, 255, 255);
	i += buttonSpacing;

	int padding = 16;
	int editorWidth = 360;
	int editorHeight = 360;
	int editorButtonWidth = (editorWidth - padding) / 2;
	UIEditBitmap* imageEdit = new UIEditBitmap(160, 16, editorWidth, editorHeight, 16, 16);

	UIButton* saveButton = new UIButton("Save",
	160, editorHeight + padding * 2,
	editorButtonWidth, 32,
	127, 0, 0);

	UIButton* loadButton = new UIButton("Load",
	160 + editorButtonWidth + 16, editorHeight + padding * 2,
	editorButtonWidth, 32,
	127, 0, 0);
	
	UIButton* export1DButton = new UIButton("Export Array",
	160, editorHeight + padding * 2 + 40,
	editorButtonWidth, 32,
	127, 0, 0);

	UIButton* export2DButton = new UIButton("Export 2D Array",
	160 + editorButtonWidth + 16, editorHeight + padding * 2 + 40,
	editorButtonWidth, 32,
	127, 0, 0);

	auto editorToolsFunc = [toolLabel, imageEdit, clearButton, pencilButton, lineButton, eraserButton] (UIButton* button) {
		if (button == clearButton) {
			if (MessageBoxA(NULL, "Are you sure?", "Riddle me this...", MB_YESNO | MB_ICONASTERISK) == IDYES)
				imageEdit->clear();
		} else if (button == pencilButton) {
			imageEdit->setDrawOperation(0);
			toolLabel->setText("-> Pencil <-");
		} else if (button == lineButton) {
			imageEdit->setDrawOperation(2);
			toolLabel->setText("-> Line <-");
		} else if (button == eraserButton) {
			imageEdit->setDrawOperation(1);
			toolLabel->setText("-> Eraser <-");
		}
	};
	clearButton->setClickFunc(editorToolsFunc);
	pencilButton->setClickFunc(editorToolsFunc);
	lineButton->setClickFunc(editorToolsFunc);
	eraserButton->setClickFunc(editorToolsFunc);

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

	redSlider->setMaxColor(255, 0, 0);
	greenSlider->setMaxColor(0, 255, 0);
	blueSlider->setMaxColor(0, 0, 255);
	
	editorScreen->addUIWidget(toolLabel);
	editorScreen->addUIWidget(clearButton);
	editorScreen->addUIWidget(pencilButton);
	editorScreen->addUIWidget(lineButton);
	editorScreen->addUIWidget(eraserButton);
	editorScreen->addUIWidget(redSlider);
	editorScreen->addUIWidget(greenSlider);
	editorScreen->addUIWidget(blueSlider);
	editorScreen->addUIWidget(colorDisplay);
	editorScreen->addUIWidget(imageEdit);
	editorScreen->addUIWidget(saveButton);
	editorScreen->addUIWidget(loadButton);
	editorScreen->addUIWidget(export1DButton);
	editorScreen->addUIWidget(export2DButton);

	winapi_show();

	while (winapi_run()) {
		unsigned char r = redSlider->getValue();
		unsigned char g = greenSlider->getValue();
		unsigned char b = blueSlider->getValue();
		imageEdit->setDrawColor(r, g, b);
		colorDisplay->setColor(r, g, b);
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