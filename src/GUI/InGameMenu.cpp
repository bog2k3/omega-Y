#include "InGameMenu.h"

#include <boglfw/GUI/ListLayout.h>
#include <boglfw/GUI/controls/Button.h>

InGameMenu::InGameMenu() {
	setSize({100, 100, FlexCoord::PERCENT});
	setClientArea({400, 50}, {400, 50});
	auto layout = std::make_shared<ListLayout>();
	layout->setItemSpacing(30);
	layout->setAlignment(ListLayout::CENTER);			// center items horizontally
	layout->setVerticalAlignment(ListLayout::MIDDLE);	// center contents vertically
	useLayout(layout);

	addButton("Resume", onBack);
	addButton("Quit", onQuit);
}

void InGameMenu::addButton(const char* text, Event<void()> &onClick) {
	auto pBut = std::make_shared<Button>(text);
	pBut->onClick.forward(onClick);
	addElement(pBut);
}
