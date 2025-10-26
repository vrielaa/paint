#include "imgui.h"
#include "imgui-SFML.h"

#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cstdint>

#include <algorithm>

struct Line {
    sf::Vector2f start;
    sf::Vector2f end;
    sf::Color color;
    float thickness;
};
enum ToolType {
    TOOL_LINE,
    TOOL_RECTANGLE,
    TOOL_FILLED_RECTANGLE,
    TOOL_CIRCLE
};
class PaintApp {
    public:

	int selectedTool = TOOL_LINE;

	bool isToolsShown = true;
	bool isDrawingLine = false;
	std::vector<Line> lines;
	sf::Vector2f lineStart{}, lineEnd{}; //punkty startu i koñca linii, która jest aktualnie rysowana
	sf::Color currentColor = sf::Color::Black;
    PaintApp() {
        // Konstruktor aplikacji Paint
    }
    void run() {
        // G³ówna pêtla aplikacji Paint
    }

    void menuBar(sf::RenderWindow& window);
    void newFile(sf::RenderWindow& window);
    void openFile(const std::string& filename);
    void saveFileAs(const std::string& filename);
    void exit();

    void help();
	void drawToolsWindow(sf::RenderWindow& window);
    void keepImGuiWindowInside(const sf::RenderWindow& sfWindow, float margin = 0.0f);


	//narzedzia do rysowania
	void lineTool(sf::RenderWindow& window);
	void rectangleTool();
	void filledRectangleTool();
	void circleTool();

	void chosenTool(sf::RenderWindow& window);
	void renderCanvas(sf::RenderWindow& window);

};

//("label", "shortcut", selected, enabled)
//selected - czy element jest zaznaczony (ptaszek np. dla opcji prze³¹czanych)
//enabled - czy element jest aktywny (jeœli false, jest wyszarzony i nieklikalny)

void PaintApp::menuBar(sf::RenderWindow& window) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N", false, true)) {
				newFile(window);
            }
            if (ImGui::MenuItem("Open...", "Ctrl+O", false, true)) {
				//openFile();
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+S", false, true)) {
				//saveFileAs();
            }
            if (ImGui::MenuItem("Exit", "Alt+F4", false, true)) {
				//exit();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Show Tool Options", "", isToolsShown)) { //isToolsShown - przekazanie zmiennej, ktora bedzie decydowac o tym, czy wyswietlic okno z narzedziami
				isToolsShown = !isToolsShown; //domyslnie true, wiec po pierwszym kliknieciu zmieni na false, a po drugim znow na true i tak dalej
				
            }
			ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About", "", false, true)) {
				//help();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void PaintApp::keepImGuiWindowInside(const sf::RenderWindow& sfWindow, float margin) {

    //--  zapobieganie wychodzeni okna poza obszar ekranu  --

    ImVec2 toolPosition = ImGui::GetWindowPos(); //pobranie aktualnej pozycji tool window
    ImVec2 toolSize = ImGui::GetWindowSize(); //pobranie aktualnego rozmiaru tool window
    sf::Vector2u windowSizeSFML = sfWindow.getSize(); //pobranie rozmiaru okna SFML

    float targetToolWidth = std::min(toolSize.x, static_cast<float>(windowSizeSFML.x));
    float targetToolHeight = std::min(toolSize.y, static_cast<float>(windowSizeSFML.y));

    // Poprawiamy rozmiar okna narzêdzi, jeœli jest wiêkszy ni¿ rozmiar okna SFML (mozna zmieniac po jednej osi, wiêc sprawdzamy osobno szerokoœæ i wysokoœæ)

    if (toolSize.x != targetToolWidth) {
        ImGui::SetWindowSize(ImVec2(targetToolWidth - margin, toolSize.y));
    }

    if (toolSize.y != targetToolHeight) {
        ImGui::SetWindowSize(ImVec2(toolSize.x, targetToolHeight - margin));
    }

    toolSize = ImGui::GetWindowSize(); //ponowne pobranie rozmiaru okna narzêdzi po ewentualnej zmianie
    toolPosition = ImGui::GetWindowPos(); //ponowne pobranie pozycji okna narzêdzi po ewentualnej zmianie




    float maxX = std::max(0.0f, static_cast<float>(windowSizeSFML.x) - toolSize.x); //maksymalna dozwolona pozycja X tool window
    float maxY = std::max(0.0f, static_cast<float>(windowSizeSFML.y) - toolSize.y); //maksymalna dozwolona pozycja Y tool window
    ImVec2 clampedPos = ImVec2(  //obliczenie poprawnej pozycji okna narzêdzi
        std::clamp(toolPosition.x, 0.0f, maxX), //std::clamp - funkcja z C++17, która ogranicza wartoœæ do podanego zakresu
        std::clamp(toolPosition.y, 0.0f, maxY)
    );

    if (clampedPos.x != toolPosition.x || clampedPos.y != toolPosition.y) {
        ImGui::SetWindowPos(clampedPos); //ustawienie poprawnej pozycji okna narzêdzi, jeœli by³a poza ekranem
    }
    //--  koniec zapobiegania wychodzeni okna poza obszar ekranu  --
}


void PaintApp::drawToolsWindow(sf::RenderWindow& window) {
	if (!isToolsShown) return;
    ImGui::SetNextWindowPos(ImVec2(20, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(260, 200), ImGuiCond_FirstUseEver);
	float margin = 25.0f;
    //-- okno z narzedziami --
    if (ImGui::Begin("Tools", &isToolsShown)) { //&isToolsShown - przekazanie referencji do zmiennej, aby okno mog³o byæ zamkniête

		keepImGuiWindowInside(window, margin); //zapobieganie wychodzeni okna poza obszar ekranu
		static float brushSize = 12.0f;

		ImGui::TextUnformatted("Tool Options"); //TextUnformatted - wyœwietla tekst bez dodatkowego formatowania
		ImGui::Separator(); //Separator - rysuje poziom¹ liniê oddzielaj¹c¹ elementy interfejsu

		ImGui::RadioButton("Line", &selectedTool, TOOL_LINE); //RadioButton - przycisk radiowy do wyboru narzêdzia; selectedTool - zmienna przechowuj¹ca aktualnie wybrane narzêdzie, TOOL_LINE - wartoœæ przypisana do tego narzêdzia; jesli selectedTool równa siê TOOL_LINE, to ten przycisk bêdzie zaznaczony, a jeœli uzytkownik kliknie na inny przycisk, to selectedTool zostanie ustawione na wartoœæ przypisan¹ do tego przycisku
		ImGui::RadioButton("Rectangle", &selectedTool, TOOL_RECTANGLE);
		ImGui::RadioButton("Filled Rectangle", &selectedTool, TOOL_FILLED_RECTANGLE);
		ImGui::RadioButton("Circle", &selectedTool, TOOL_CIRCLE);
        ImGui::SliderFloat("Brush Size", &brushSize, 1.0f, 100.0f); //SliderFloat - suwak do wyboru wartoœci zmiennoprzecinkowej

		static float color[3] = { 1.f,0.2f,0.3f };
		ImGui::ColorEdit3("Brush Color", color); //ColorEdit3 - edytor koloru RGB, color - domyœlny kolor
        currentColor = sf::Color(
            static_cast<uint8_t>(color[0] * 255.f),
            static_cast<uint8_t>(color[1] * 255.f),
            static_cast<uint8_t>(color[2] * 255.f)
        );

    }
    ImGui::End();
	//-- koniec okna z narzedziami --

	
}


void PaintApp::chosenTool(sf::RenderWindow& window) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return; //jeœli imgui  (czyli okno tools) przechwyci³o myszkê, nie rysuj linii (bo takto rysowa³oby siê w tle, pod oknem imgui)
    }
    switch (selectedTool)
    {
        case TOOL_LINE:
        lineTool(window);
		break;
        case TOOL_RECTANGLE:
        rectangleTool();
        break;
        case TOOL_FILLED_RECTANGLE:
        //filledRectangleTool();
        break;
        case TOOL_CIRCLE:
        //circleTool();
		break;
    default:
        break;
    }

}
void PaintApp::lineTool(sf::RenderWindow& window) { 
	if (selectedTool != TOOL_LINE) return; //jeœli wybrane narzêdzie to nie linia, zakoñcz funkcjê; w petli w main co klatkê wywo³ywane s¹ wszystkie narzêdzia, wiêc ka¿de narzêdzie musi sprawdziæ, czy jest aktualnie wybrane
	

	sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window)); //pobranie pozycji myszy w oknie SFML w momencie wywo³ania funkcji

	static bool previousMouseState = false; //poprzedni stan przycisku myszy
	bool pressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left); //aktualny stan przycisku myszy

    if (!isDrawingLine) {
        if (pressed && !previousMouseState) { //jeœli przycisk myszy jest wciœniêty i wczeœniej nie by³ wciœniêty, rozpocznij rysowanie linii
            isDrawingLine = true;
            lineStart = lineEnd = mouse; //ustawienie punktu startu i koñca linii na aktualn¹ pozycjê myszy

        }
    }
	else { //jeœli ju¿ rysujemy liniê, aktualizuj punkt koñcowy linii
         

            lineEnd = mouse;

			// koniec: puszczono LMB , zapisz liniê do wektora lines
            if (!pressed && previousMouseState) {
                Line L;
                L.start = lineStart;
                L.end = lineEnd;
                L.color = currentColor;
                L.thickness = 1.0f; // tu na razie sta³a gruboœæ 1px
                lines.push_back(L);
                isDrawingLine = false;
            }
    }
    

		previousMouseState = pressed; //aktualizacja poprzedniego stanu przycisku myszy
  
    


}

void PaintApp::rectangleTool() {
	if (selectedTool != TOOL_RECTANGLE) return;
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return; //jeœli imgui  (czyli okno tools) przechwyci³o myszkê, nie rysuj linii (bo takto rysowa³oby siê w tle, pod oknem imgui)
    }

}

void PaintApp::renderCanvas(sf::RenderWindow& window) {
    // narysuj zapisane linie
    for (const auto& line : lines) {
        sf::Vertex v[2];
        v[0].position = line.start;
        v[0].color = line.color;
        v[1].position = line.end;
        v[1].color = line.color;
        window.draw(v, 2, sf::PrimitiveType::Lines);
    }

    // podgl¹d aktualnie rysowanej linii
    if (isDrawingLine) {
        sf::Vertex v[2];
        v[0].position = lineStart;
        v[0].color = currentColor;
        v[1].position = lineEnd;
        v[1].color = currentColor;
        window.draw(v, 2, sf::PrimitiveType::Lines);
    }
}

void PaintApp::newFile(sf::RenderWindow& window) {
	window.clear(sf::Color::White); //czyœci okno na bia³o
}


int main() {
    sf::RenderWindow window(sf::VideoMode({ 1800, 900 }), "ImGui + SFML");
	sf::Clock deltaClock; //do odmierzania czasu miedzy klatkami
	PaintApp app;
	

    window.setFramerateLimit(60);
	std::ignore = ImGui::SFML::Init(window); //wskazanie gdzie rysowac imgui

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>()) window.close();
         
        }

        ImGui::SFML::Update(window, deltaClock.restart());
		app.chosenTool(window);

        app.menuBar(window);
		app.drawToolsWindow(window);
        window.clear();
		app.renderCanvas(window);
        ImGui::SFML::Render(window);
        window.display();
    }
    ImGui::SFML::Shutdown();
    return 0;
};


