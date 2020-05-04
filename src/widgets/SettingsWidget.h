#pragma once

///
/// Виджет - основной визуальный элемент на экране.
/// Он отрисовывает себя, а также может содержать другие виджеты.
///
class SettingsWidget : public GUI::Widget
{
public:
	SettingsWidget(const std::string& name, rapidxml::xml_node<>* elem);

	void Draw() override;	

private:	
	
	
};
