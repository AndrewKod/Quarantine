#pragma once

///
/// ������ - �������� ���������� ������� �� ������.
/// �� ������������ ����, � ����� ����� ��������� ������ �������.
///
class SettingsWidget : public GUI::Widget
{
public:
	SettingsWidget(const std::string& name, rapidxml::xml_node<>* elem);

	void Draw() override;	

private:	
	
	
};
