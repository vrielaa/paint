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
	sf::Vector2f lineStart{}, lineEnd{}; //punkty startu i ko�ca linii, kt�ra jest aktualnie rysowana
	sf::Color currentColor = sf::Color::Black;
    PaintApp() {
        // Konstruktor aplikacji Paint
    }
    void run() {
        // G��wna p�tla aplikacji Paint
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
//selected - czy element jest zaznaczony (ptaszek np. dla opcji prze��czanych)
//enabled - czy element jest aktywny (je�li false, jest wyszarzony i nieklikalny)

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

    // Poprawiamy rozmiar okna narz�dzi, je�li jest wi�kszy ni� rozmiar okna SFML (mozna zmieniac po jednej osi, wi�c sprawdzamy osobno szeroko�� i wysoko��)

    if (toolSize.x != targetToolWidth) {
        ImGui::SetWindowSize(ImVec2(targetToolWidth - margin, toolSize.y));
    }

    if (toolSize.y != targetToolHeight) {
        ImGui::SetWindowSize(ImVec2(toolSize.x, targetToolHeight - margin));
    }

    toolSize = ImGui::GetWindowSize(); //ponowne pobranie rozmiaru okna narz�dzi po ewentualnej zmianie
    toolPosition = ImGui::GetWindowPos(); //ponowne pobranie pozycji okna narz�dzi po ewentualnej zmianie




    float maxX = std::max(0.0f, static_cast<float>(windowSizeSFML.x) - toolSize.x); //maksymalna dozwolona pozycja X tool window
    float maxY = std::max(0.0f, static_cast<float>(windowSizeSFML.y) - toolSize.y); //maksymalna dozwolona pozycja Y tool window
    ImVec2 clampedPos = ImVec2(  //obliczenie poprawnej pozycji okna narz�dzi
        std::clamp(toolPosition.x, 0.0f, maxX), //std::clamp - funkcja z C++17, kt�ra ogranicza warto�� do podanego zakresu
        std::clamp(toolPosition.y, 0.0f, maxY)
    );

    if (clampedPos.x != toolPosition.x || clampedPos.y != toolPosition.y) {
        ImGui::SetWindowPos(clampedPos); //ustawienie poprawnej pozycji okna narz�dzi, je�li by�a poza ekranem
    }
    //--  koniec zapobiegania wychodzeni okna poza obszar ekranu  --
}


void PaintApp::drawToolsWindow(sf::RenderWindow& window) {
	if (!isToolsShown) return;
    ImGui::SetNextWindowPos(ImVec2(20, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(260, 200), ImGuiCond_FirstUseEver);
	float margin = 25.0f;
    //-- okno z narzedziami --
    if (ImGui::Begin("Tools", &isToolsShown)) { //&isToolsShown - przekazanie referencji do zmiennej, aby okno mog�o by� zamkni�te

		keepImGuiWindowInside(window, margin); //zapobieganie wychodzeni okna poza obszar ekranu
		static float brushSize = 12.0f;

		ImGui::TextUnformatted("Tool Options"); //TextUnformatted - wy�wietla tekst bez dodatkowego formatowania
		ImGui::Separator(); //Separator - rysuje poziom� lini� oddzielaj�c� elementy interfejsu

		ImGui::RadioButton("Line", &selectedTool, TOOL_LINE); //RadioButton - przycisk radiowy do wyboru narz�dzia; selectedTool - zmienna przechowuj�ca aktualnie wybrane narz�dzie, TOOL_LINE - warto�� przypisana do tego narz�dzia; jesli selectedTool r�wna si� TOOL_LINE, to ten przycisk b�dzie zaznaczony, a je�li uzytkownik kliknie na inny przycisk, to selectedTool zostanie ustawione na warto�� przypisan� do tego przycisku
		ImGui::RadioButton("Rectangle", &selectedTool, TOOL_RECTANGLE);
		ImGui::RadioButton("Filled Rectangle", &selectedTool, TOOL_FILLED_RECTANGLE);
		ImGui::RadioButton("Circle", &selectedTool, TOOL_CIRCLE);
        ImGui::SliderFloat("Brush Size", &brushSize, 1.0f, 100.0f); //SliderFloat - suwak do wyboru warto�ci zmiennoprzecinkowej

		static float color[3] = { 1.f,0.2f,0.3f };
		ImGui::ColorEdit3("Brush Color", color); //ColorEdit3 - edytor koloru RGB, color - domy�lny kolor
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
        return; //je�li imgui  (czyli okno tools) przechwyci�o myszk�, nie rysuj linii (bo takto rysowa�oby si� w tle, pod oknem imgui)
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
	if (selectedTool != TOOL_LINE) return; //je�li wybrane narz�dzie to nie linia, zako�cz funkcj�; w petli w main co klatk� wywo�ywane s� wszystkie narz�dzia, wi�c ka�de narz�dzie musi sprawdzi�, czy jest aktualnie wybrane
	

	sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window)); //pobranie pozycji myszy w oknie SFML w momencie wywo�ania funkcji

	static bool previousMouseState = false; //poprzedni stan przycisku myszy
	bool pressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left); //aktualny stan przycisku myszy

    if (!isDrawingLine) {
        if (pressed && !previousMouseState) { //je�li przycisk myszy jest wci�ni�ty i wcze�niej nie by� wci�ni�ty, rozpocznij rysowanie linii
            isDrawingLine = true;
            lineStart = lineEnd = mouse; //ustawienie punktu startu i ko�ca linii na aktualn� pozycj� myszy

        }
    }
	else { //je�li ju� rysujemy lini�, aktualizuj punkt ko�cowy linii
         

            lineEnd = mouse;

			// koniec: puszczono LMB , zapisz lini� do wektora lines
            if (!pressed && previousMouseState) {
                Line L;
                L.start = lineStart;
                L.end = lineEnd;
                L.color = currentColor;
                L.thickness = 1.0f; // tu na razie sta�a grubo�� 1px
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
        return; //je�li imgui  (czyli okno tools) przechwyci�o myszk�, nie rysuj linii (bo takto rysowa�oby si� w tle, pod oknem imgui)
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

    // podgl�d aktualnie rysowanej linii
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
	window.clear(sf::Color::White); //czy�ci okno na bia�o
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


