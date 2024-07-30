#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#define ANSI_COLOR_RED "\e[0;31m" 
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_GREEN_BOLD "\e[1;32m"
#define ANSI_COLOR_RED_BOLD "\e[1;31m"
#define ANSI_COLOR_YELLOW_BOLD "\e[1;33m"
#define ANSI_COLOR_CYAN "\e[0;36m"
#define ANSI_COLOR_PURPLE "\e[0;35m"


char* mealNames[4] =  {"BREAKFAST", "LUNCH", "DINNER", "SNACKS"};
// This variable is used for the sake of not printing too many things at once
// when the user needs to press any key to continue the program, the character is stored in this variable
char cont[127];

typedef struct Food {
    char name[100];
    char portion[20];
    float calories;
    int lineNumber;
    char category[30];
} Food;

typedef struct Meal {
    Food *food; // Array of Food structs
    int memAllocated;
    int numElements;
} Meal;

// initialise a struct for breakfast, lunch, dinner, snacks
// NOTE THAT PEOPLE CAN EAT MULTIPLE food items FOR EACH OF THESE meals, so they are all dynmaic arrays 
typedef struct TodayMeals {
    Meal breakfast;
    Meal lunch;
    Meal dinner;
    Meal snacks;
} TodayMeals;

typedef struct CalorieAim {
    int calorieTarget;
    char isGainingWeight;
} CalorieAim;

typedef struct FoodHistoryItem {
    char date[15];
    int mealType;
    int foodId;
} FoodHistoryItem;


typedef struct FoodHistoryItemArr {
    FoodHistoryItem *items; // Array of food structs
    int memAllocated;
    int numElements;
} FoodHistoryItemArr;


void FileRead(char *searchterm, Food **structs, int *memAllocated, int *numElements);

// going up one line, clearing that line, then going to the start 2 times
void toLowerCase(char *str);

void moveCursor(int num);

// Initialises a Meal struct
void initMeal(Meal *meal);

// initialises a dynamic arr of integers
void initFoodHistoryItemArr(FoodHistoryItemArr *mealIDs);

int outputCurrentMeal(Meal currentMeal);

void outputCalorieIntake(TodayMeals todayMeals, CalorieAim calorieTarget, char *currentDate);

// Appends food into a dynamic array of type Food
// The reason why I used a double pointer for structs is so that reallocation need not be done within one function, it can be done in others
// reallocation requires assigning of variables, but i dont want to make this struct a global variable
void appendFood(Food **structs, int *memAllocated, int *numElements, Food food);

void appendFoodHistoryItem(FoodHistoryItem **arr, int *memAllocated, int *numElements, FoodHistoryItem item);

void writeMealsFile(TodayMeals todayMeals, char* date);

void askCalorieDay(CalorieAim userCalorieAim, TodayMeals todayMeals, char *currentDate);

void mainMenuPrompt(void);

void writeCalorieAim(CalorieAim userCalorieAim);

FoodHistoryItemArr searchFoodInfoAtDate(char *date);

CalorieAim getCalorieTarget(void);

Food getFoodFromID(int num);

TodayMeals getMealsFromDate(char currentDate[11]);

void createNewFood(void);

int isAsciiString(const char *str);

void strScanf(char *question, char *var);

int isInt(char* str);

void intScanf(char *question, int *var, int repeated);

void createMealsFile(vofid);

void createHistoryFile(void);

void outputTable(TodayMeals *weekMeals, char *dates[], int numDates);

void searchPastCalorieIntake(CalorieAim userCalorieAim);

time_t parse_date(const char *date_str);

void insertFoods(TodayMeals todayMeals, char* dateToOverride);

void main(void) {
    printf("----------WELCOME TO THE FOOD NUTRITION CALCULATOR----------\n\n");
    
    // FROM THIS POINT IS THE MAIN MENU
    int mainMenuOption = 0; // Chosen option for main menu
    
    // while (mainMenuOption != 1) {
    CalorieAim userCalorieAim = getCalorieTarget();
    
    if (userCalorieAim.calorieTarget == 0) {
        printf("We currently do not have your calorieTarget information.\n");
        // TODO: put this part in a function
        // TODO: if you want you can make this 3 options: 1800 kcal, 2200 kcal, custom
        printf("What is your target calorie intake? (Enter in kcal as a whole number): ");
        scanf("%d", &userCalorieAim.calorieTarget);
        moveCursor(1);
        printf("What is your target calorie intake? (Enter in kcal as a whole number): " ANSI_COLOR_GREEN_BOLD "%d" ANSI_COLOR_RESET "\n", userCalorieAim.calorieTarget);
        

        printf("Are you trying to gain weight? (y/n), n for trying to lose weight: ");
        scanf(" %c", &userCalorieAim.isGainingWeight);
        moveCursor(1);
        printf("What is your calorie target? (Enter in kcal as a whole number): " ANSI_COLOR_GREEN_BOLD "%c" ANSI_COLOR_RESET "\n", userCalorieAim.isGainingWeight);

        writeCalorieAim(userCalorieAim);
    }

    // These two lines get the current time (no idea how it works as of now)
    time_t now = time(NULL); // Get the current calendar time (seconds since Epoch)
    struct tm *tm = localtime(&now); // converts it to local time format

    // tm_year is years since 1900, tm_month is 0 to 11
    char currentDate[11];
    sprintf(currentDate, "%d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);

    TodayMeals mealsAtDate = getMealsFromDate(currentDate);
    // as long as one of them have no elements, that means there has been no prior information beforehand
    // printf("%d, %d, %d, %d", mealsAtDate.breakfast.numElements, mealsAtDate.lunch.numElements, mealsAtDate.dinner.numElements, mealsAtDate.snacks.numElements);
    if (mealsAtDate.breakfast.numElements != 0 || mealsAtDate.lunch.numElements != 0 || mealsAtDate.dinner.numElements != 0 || mealsAtDate.snacks.numElements != 0) {
        outputCalorieIntake(mealsAtDate, userCalorieAim, currentDate);
    } else {
        char editCalorieIntake[127];
        printf("We currently do not know your calorie intake for today.\n");
        strScanf("Do you want to enter your calorie intake now? (enter y to enter calorie intake, any other key to go to main menu): ", editCalorieIntake);

        if (strcmp(editCalorieIntake, "y") == 0) {
            system("cls");
            TodayMeals todayMeals;
            Meal* meals[4] =  {&todayMeals.breakfast, &todayMeals.lunch,  &todayMeals.dinner,  &todayMeals.snacks};
            int initMealCount = 0;
            while (initMealCount < 4) {
                initMeal(meals[initMealCount]);
                initMealCount++;
            }
            time_t t = time(NULL); // Get the current calendar time (seconds since Epoch)
            struct tm tm = *localtime(&t); // converts it to local time format
            char currentDate[11];
            // tm_year is years since 1900, tm_month is 0 to 11
            sprintf(currentDate, "%d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

            askCalorieDay(userCalorieAim, todayMeals, currentDate);
        }
    }
    while (mainMenuOption != 1) {
        mainMenuPrompt();

        intScanf("Enter your option here (1-5): ",&mainMenuOption, 0);
        
        while (mainMenuOption < 1 && 5 < mainMenuOption) {
            printf(ANSI_COLOR_RED "%d is an invalid option. Please re-enter your input by running the program again.\n\n" ANSI_COLOR_RESET, mainMenuOption);
            intScanf("Enter your option here (1-5): ",&mainMenuOption, 0);
        }

        if (mainMenuOption == 1) {
            printf("Goodbye! Have a nice day!");

        } else if (mainMenuOption == 2) {

            system("cls");
            int optionChoice;
            intScanf("What do you want to view?\n" ANSI_COLOR_CYAN "(1)" ANSI_COLOR_RESET " Today's records\n"ANSI_COLOR_CYAN"(2)"ANSI_COLOR_RESET" Records for the past N days (Where you set N to a number)\n" ANSI_COLOR_CYAN, &optionChoice, 0);
            
            // validation
            while (optionChoice != 1 && optionChoice != 2) {
                printf(ANSI_COLOR_RED "Invalid option. Please try again." ANSI_COLOR_RESET);
                moveCursor(1);
                intScanf("", &optionChoice, 0);
            }

            if (optionChoice == 1) {
                TodayMeals mealsAtDate = getMealsFromDate(currentDate);
                if (mealsAtDate.breakfast.numElements == 0 && mealsAtDate.lunch.numElements == 0 && mealsAtDate.dinner.numElements == 0 && mealsAtDate.snacks.numElements == 0) {
                    char editCalorieIntake[127];
                    printf(ANSI_COLOR_RESET "We currently do not know your calorie intake for today.\n");
                    // printf("Do you want to enter your calorie intake now? (y/n): ");
                    strScanf("Do you want to enter your calorie intake now? (y/n): ", editCalorieIntake);
                    if (strcmp(editCalorieIntake, "y") == 0) {
                        system("cls");
                        TodayMeals todayMeals;
                        Meal* meals[4] =  {&todayMeals.breakfast, &todayMeals.lunch,  &todayMeals.dinner,  &todayMeals.snacks};
                        int initMealCount = 0;
                        while (initMealCount < 4) {
                            initMeal(meals[initMealCount]);
                            initMealCount++;
                        }
                        time_t t = time(NULL); // Get the current calendar time (seconds since Epoch)
                        struct tm tm = *localtime(&t); // converts it to local time format
                        char currentDate[11];
                        // tm_year is years since 1900, tm_month is 0 to 11
                        sprintf(currentDate, "%d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

                        askCalorieDay(userCalorieAim, todayMeals, currentDate);
                    }
                    continue;
                }
                outputCalorieIntake(mealsAtDate, userCalorieAim, currentDate);
            } else if (optionChoice == 2) {
                // weekMeals[0] is current date, at index 1 is yesterday, etc.
                searchPastCalorieIntake(userCalorieAim);
            }
            
        } else if (mainMenuOption == 3) {
            // Creating new template
            createNewFood();
        } else if (mainMenuOption == 4) {
            // Searching in the food database
            Meal meal;
            char searchterm[127] = "";
            
            while (strcmp(searchterm, "0") != 0) {
                initMeal(&meal);
                FileRead(searchterm, &(meal.food), &(meal.memAllocated), &(meal.numElements));

                strScanf("Enter anything to search in the database again, enter 0 to exit: ", searchterm);
            }

        }

    }
    
}

void writeMealsFile(TodayMeals todayMeals, char* date) {
    FILE* fptr;
    int i = 0, j = 0;
    Meal meals[4] = {todayMeals.breakfast, todayMeals.lunch,  todayMeals.dinner,  todayMeals.snacks};

    fptr = fopen("history.csv", "a");
    if (fptr == NULL) {
        perror("Error opening file");
        exit(1);
    }

    printf("file opened successfully\n");
    while (i < 4) {
        while (j < meals[i].numElements) {
            char writeText[200];

            sprintf(writeText, "%s,%d,%05d\n", date, i, meals[i].food[j].lineNumber);
            
            // NOTE: line number of 0 means food doesnt exist
            fwrite(writeText, sizeof(char), strlen(writeText), fptr);
            j++;
        }
        i++;
        j = 0;
    }

    fclose(fptr);
}
void createMealsFile(void) {
    FILE* fptr;
    char *writeText = "Desserts,Portion,Calories (kcal)\nIce Kachang,1 bowl,257\nChendol (with coconut & evaporated milk),1 bowl,593\nBubor Cha Cha,1 bowl,390\nPulut Hitam with coconut milk,1 bowl,290\nTau Suan with you tiao,1 bowl,293\nCheng Tng,1 bowl,257\nRed Bean Soup,1 bowl,278\nGreen Bean Soup,1 bowl,237\nHerbal Jelly,1 bowl,71\n\nBread,Portion,Calories (kcal)\nWhite Bread,1 slice,77\nWhole meal Bread,1 slice,72\nButter,1 tsp,38\n\"Margarine, reduced fat\",1 tsp,18\n\"Doughnut, sugared\",1 piece,165\nDoughnut,1 piece,304\nEgg Tart,1 piece,177\nApple Pie,1 piece,260\nRoti Prata (plain),1 piece,122\nRoti Prata with egg,1 piece,289\nRoti John,1 piece,571\nYou Tiao,1 piece,123\nChar Siew Pau,1 bun,212\nChicken Pau,1 bun,204\nPork Pau,1 bun,246\nHam Chim Peng (red bean filling),1 piece,273\nMee Chiang Kueh,1 piece,186\nSoon Kueh,1 piece,98\nSiew Mai,1 piece,105\nChwee Kuay,1 whole,57\nChee Cheong Fun,1 piece,133\nPopiah,1 piece,92\nCarrot Cake (mashed & fried),1 plate,467\nOyster Omelette,1 plate,650\nChicken Curry Puff,1 piece,246\nYam Cake,1 piece,174\nFish Otah,1 piece,18\nChicken Satay,1 stick,35\nChinese Rojak,1 plate,443\nIndian Rojak,1 plate,756\nFrench Fries,1 plate,450\nFried Chicken Wing,1 piece,109\nFried Chicken Drumstick,1 piece,207\nPotato Chips,1 plate,184\n\nDrinks,Portion,Calories (kcal)\nPlain Water,1 cup,0\nTea with milk & sugar,1 cup,59\nCoffee with milk & sugar,1 cup,59\nFresh Milk,1 cup,163\nLow Fat Milk,1 cup,102\nOrange Juice,1 cup,80\nSugar Cane Juice,1 cup,158\nSoft Drink,1 can,120\nBandung,1 glass,153\nSoya Bean Milk (sweetened),1 cup,163\nBarley Water,1 cup,55\nBubble Tea with milk,1 glass,232\nBubble Tea with milk & pearls,1 glass,340\nBeer,1 glass,128\nWine (Red / White),1 glass,122\n\nNoodles,Portion,Calories (kcal)\nMee Rebus,1 plate,558\nMee Siam,1 plate,519\nMee Soto,1 bowl,432\nMee Goreng,1 plate,660\nNoodles with Minced Pork & Mushroom (Dry),1 bowl,511\n\"Ban Mian (with egg, meat & veggies)\",1 bowl,475\nFried Kway Teow (with cockles),1 plate,743\nHor Fun,1 plate,708\nLaksa,1 bowl,589\nFried Beehoon (plain),1 plate,252\nLor Mee,1 bowl,383\n\"Fishball Noodles, Dry\",1 bowl,364\n\"Fishball Noodles, Soup\",1 bowl,330\nFish Head Beehoon Soup,1 bowl,666\nFish Slice Beehoon Soup,1 bowl,349\nFried Hokkien Prawn Mee,1 plate,615\n\"Prawn Noodles, Dry\",1 bowl,459\n\"Prawn Noodles, Soup\",1 bowl,293\n\"Wanton Noodles, Dry\",1 bowl,409\n\"Wanton Noodles, Soup\",1 bowl,217\n\"Kway Chap (with meat, intestine & taupok)\",1 bowl,648\n\nRice,Portion,Calories (kcal)\nChicken Porridge,1 bowl,214\nFish Porridge,1 bowl,261\nPork Porridge,1 bowl,362\nCentury Egg Porridge,1 bowl,422\nChar Siew Rice,1 plate,600\nDuck Rice,1 plate,706\nChicken Rice,1 plate,702\nFried Rice,1 plate,508\nNasi Lemak,1 plate,600\nNasi Briyani with Chicken,1 plate,880\nBeef Rendang,1 plate,228\nRice,1 bowl,260\n\nSoup,Portion,Calories (kcal)\nBak Kut Teh,1 bowl,342\nYong Tauhu Soup,1 bowl,49\nHerbal Black Chicken Soup,1 bowl,186\nPig Organ Soup,1 bowl,158\nTom Yam Seafood Soup,1 bowl,271\nWatercress & Pork Ribs Soup,1 bowl,92\nMutton Soup,1 bowl,285\nCurry Laksa,1 bowl,589\nChicken Curry,1 bowl,450\nFish Head Curry,1 plate,288\nCream of Mushroom,1 bowl,132\n\nWestern,Portion,Calories (kcal)\nFish & Chips,1 serving,848\nPizza,130g,299\nSpaghetti with Meat Sauce,1 plate,240\nHamburger,1 burger,252\nMcDouble,1 burger,392\nBig Mac,1 burger,522\nCheeseburger,1 burger,300\nDouble Cheeseburger,1 burger,440\nMcChicken,1 burger,385\nMcSpicy,1 burger,522\nDouble McSpicy,1 burger,800\nFilet-O-Fish,1 burger,332\nDouble Filet-O-Fish,1 burger,474\nQuarter Pounder with Cheese,1 burger,532\nDouble Quarter Pounder with Cheese,1 burger,768\nChicken McNuggets,6 pieces,303\nHotcakes,3 pieces,557\nHotcakes with Sausage,3 pieces,640\nSpicy Chicken McWrap,1 wrap,544\n\nMcDonald,Portion,Calories (kcal)\nBig Breakfast,1 set,525\nBreakfast Deluxe,1 set,826\nBreakfast Wrap Chicken Ham,1 set,429.8\nBreakfast Wrap Chicken Sausage,1 set,493\nWholegrain Chicken Muffin,1,401\nWholegrain Egg McMuffin,1,288\nHotcakes,3 pieces,557\nHotcakes with Sausage,1 set,640\nHotcakes (2pc) Happy Meal,1 set,442\nWholegrain Sausage McMuffin,1,277\nWholegrain Sausage McMuffi with Egg,1,356\nHamburger,1,252\nBig Mac,1,522\nCheeseburger,1,300\nDouble Cheeseburger,1,440\nFilet-O-Fish,1,332\nDouble Filet-O-Fish,1,474\nMcChicken,1,385\nMcSpicy,1,522\nDouble McSpicy,1,800\nChicken McNuggets (4pc) Happy Meal,1 set,226\nChicken McNuggets - 6pc,1 box,303\nChicken McNuggets - 9pc,1 box,454\nChicken McNuggets - 20pc,1 box,1009\nMcWings,1 box,498\nHappy Sharing Box A,1 box,1602\nHappy Sharing Box B,1 box,801\nGarden Side Salad,1 bowl,21\nSkinless Grilled Chicken Salad,1 plate,161\nSkinless Grilled Chicken McWrap (Wholegrain),1 wrap,337.6\nRed Rice Porridge,1 bowl,403\nApple Slices,1 pack,30\nCorn Cup - 3oz,1 cup,66\nCorn Cup - 4.5oz,1 cup,100\nHashbrown,1 piece,149\nApple Pie,1,247\nCinnamon Melts,1,480\nVanilla Cone,1 cone,147\nChocoCone,1 cone,295.6\nHot Fudge Sundae,1 cup,343\nStrawberry Sundae,1 cup,285\nMcFlurry Oreo,1 cup,402\nMudPie McFlurry,1 cup,532\nStrawberry Shortcake McFlurry,1 cup,474\nVanilla Shake,1 cup,397\nStrawberry Shake,1 cup,400\nChocolate Shake,1 cup,405\nHot Tea,1 cup,4\nMcCafe Premium Roast Coffee,1 cup,14\nMcCafe Cappuccino - Small,1 cup,120\nMcCafe Cappuccino - Medium,1 cup,170\nMcCafe Latte,1 cup,186\nHot Milo,1 cup,200\nMagnolia Lo-Fat Hi-Calcium Milk,1 pack,95\nRibena Less Sweet,1 pack,74\n100% Pure Orange Juice - Small,1 glass,145\n100% Pure Orange Juice - Medium,1 glass,184\nCaramel Frappe - Regular,1 glass,454\nCaramel Frappe - Medium,1 glass,624\nCoca-Cola Original Taste Less Sugar - X-Small,1 glass,100\nCoca-Cola Original Taste Less Sugar - Small,1 glass,153\nCoca-Cola Original Taste Less Sugar - Medium,1 glass,213\nCoca-Cola Original Taste Less Sugar - Large,1 glass,305\nCoca-Cola No Sugar - X-Small,1 glass,0\nCoca-Cola No Sugar - Small,1 glass,0\nCoca-Cola No Sugar - Medium,1 glass,0\nCoca-Cola No Sugar - large,1 glass,0\nIced Lemon Tea - Small,1 glass,153\nIced Lemon Tea - Medium,1 glass,213\nIced Milo - X-Small,1 glass,120\nIced Milo - Small,1 glass,228\nIced Milo - Medium,1 glass,322\nJasmine Green Tea - Small,1 glass,95\nJasmine Green Tea - Medium,1 glass,132\nSprite - X-Small,1 glass,109\nSprite - Small,1 glass,167\nSprite - Medium,1 glass,233\nSprite - Large,1 glass,334\nDasani Drinking Water,1 bottle,0\n\nKFC,Portion,Calories (kcal)\nOriginal Recipe Chicken Drumstick,1,149\nOriginal Recipe Chicken Keel,1,304\nOriginal Recipe Chicken Rib,1,257\nOriginal Recipe Chicken Thigh,1,329\nOriginal Recipe Chicken Wing,1,157\nHot & Crispy Chicken Drumstick,1,212\nHot & Crispy Chicken Keel,1,390\nHot & Crispy Chicken Rib,1,220\nHot & Crispy Chicken Thigh,1,399\nHot & Crispy Chicken Wing,1,190\nCheesy Zinger Stacker,1 burger,1106\nZinger,1 burger,633\nShrooms Fillet Burger,1 burger,386\nColonel Burger,1 burger,451\nPockett Bandito,1 warp,622\nOriginal Recipe Rice Bowl,1 bowl,440\nCurry Rice Bowl,1 bowl,515\nFamous Potato Bowl,1 bowl,356\nOriginal Recipe Porridge,1 bowl,221\nOriginal Recipe Platter,1 plate,556\nPancakes Platter,1 plate,290\nPancakes & Bacon Platter,1 plate,320\nOriginal Recipe Twister,1 wrap,484\nAmerican Twister,1 wrap,461\nRiser,1 burger,515\nHash Brown,1 piece,102\nHot & Crispy Tenders (3 pcs),1 plate,304\nHot & Crispy Tenders (5 pcs),1 plate,507\nNuggets (6 pcs),1 box,239\nNuggets (20 pcs),1 box,796\nPopcorn Chicken,1 cup,446\nWhipped Potato (reg),1 bowl,51\nWhipped Potato (med),1 bowl,102\nColeslaw (reg),1 bowl,98\nColeslaw (med),1 bowl,197\nFrench Fries (reg),1 pack,242\nFrench Fries (med),1 pack,343\nFrench Fries (lrg),1 pack,367\nFrench Fries (reg),1 pack,592\nKFC Froyo - Natural Cone,1 cone,107\nKFC Froyo - Natural Cup,1 cup,118\nMineral Water,1 bottle,0\nMagnolia Fresh Milk - 200ml,1 pack,132\nHot Milo - 237ml,1 cup,128\nPepsi (reg) - 330ml,1 glass,155\nPepsi (med) - 470ml,1 glass,220\nPepsi Black (reg) - 330ml,1 glass,0\nPepsi Black (med) - 470ml,1 glass,0\n7 Up (reg) - 330ml,1 glass,143\n7 Up (med) - 470ml,1 glass,203\nMountain Dew (reg) - 330ml,1 glass,152\nMountain Dew (med) - 470ml,1 glass,216\nMug Root Beer (reg) - 330ml,1 glass,145\nMug Root Beer (med) - 470ml,1 glass,207\nLipton Iced Lemon Tea (reg) -330ml,1 glass,109\nLipton Iced Lemon Tea (med) - 446ml,1 glass,161\nSJORA Mango Peach - 330ml,1 glass,113\nSJORA Mango Peach (med) - 410ml,1 glass,154\nIced Milo (reg) - 310ml,1 glass,188\nIced Milo (med) - 420ml,1 glass,254\nCoffee No Sugar,1 cup,0\nTea No Sugar,1 cup,0\n\nBurger King,Portion,Calories (kcal)\nHamburger,,294.41\nDouble Cheeseburger,,360\nUltimate Angus Mushroom Swiss,,430.41\nDouble Mushroom Swiss,,620.83\nUltimate Angus Classic Beef,,457.91\nDouble BBQ Turkey Bacon,,515.98\nWestern Whopper Jr.,,431.31\nDouble Western Whopper,,770\nWhopper Jr.,,364\nWhopper,,625.83\nDouble Whopper with Cheese,,911.99\nBBQ CHICK'N CRISP,,423\nTendergril Chicken,,438.47\nMushroom Swiss Tendergrill Chicken,,473.98\nLong Chicken,,602.04\nTendercrisp Chicken,,616\nUltimate Tendercrisp Chicken,,685.27\nPancakes Platter,,236.5\nPancakes with Sausage Platter,,303\nTurkey Ham & Cheese Omelette Platter,,639.82\nTurkey Bacon CROISSAN'WICH with Egg,,366\nTurkey Ham CROISSAN'WICH with Egg,,381.36\nTurkey Sausage CROISSAN'WICH,,334.84\nTurkey Sausage CROISSAN'WICH with Egg,,424\nMushroom Swiss CROISSAN'WICH with Egg,,425\nCHICK'N CRISP CROISSAN'WICH,,514\nHash Browns (S),,136.5\nOnion Rings (S),,151\nFrench Fries (S),,183.26\nCheesy Fries,,200\nBK Nuggets (9pcs),,352\nVanilla Cone,,160\nTaro Turnover,,257\nChocolate Sundae,,280\nHershey's Sundae Pie,,310\nSalad,,11\n\nMOS Burger,Portion,Calories (kcal)\nMos Burger,,404\nMos Cheeseburger,,417\nMos Chicken,,313\nMos Vegetable Burger,,338\nMos Yasai Burger,,342\nSpicy Mos Burger,,346\nSpicy Mos Cheeseburger,,408\nChicken Burger,,378\nFish Burger,,407\nHokkaido Croquette Burger,,327\nTeriyaki Chicken Burger,,428\nFort Golden Shrimp Burger,,406\nNatsumi Beef Burger -130g,,181\nNatsumi Teriyaki Chicken Burger -120g,,221\nNatsumi Fried Shrimp Burger,,233\nHam Omelette Burger,,282\nKinpira Rice Burger,,241\nYakiniku Rice Burger,,278\nTsukune Rice Burger,,319\nKakiage Rice Burger,,356\nUnagi Rice Burger,,370\nEbi Rice Burger - 250g,,501\nNatsumi Chicken Teriyaki,,320\nNatsumi Fish,,266\nGreen Salad,1 serving,44\nCorn Soup - 150ml,1 cup,82\nClam Chowder Soup,1 cup,74\nButterfly Prawn,5 pieces,93\nChicken Nuggets,5 pieces,143\nFrence Fries Potato,1 piece,149\nFrench Fries (Small),,128\nFrench Fries (Large),,263\nApple Pie,,240\nOnion Rings,,225\nPumpkin Croquette,,276\nIced Milk Tea,,92\nMatcha Latte,,126\n\nJollibee,Portion,Calories (kcal)\nCheese Burger,1 serving,360\nYum Burger,1 serving,250\njolly Hotdog,1 serving,389\njolly Spaghetti,1 serving,560\nChicken Joy - Breast,1 piece,320\nChicken Joy - Drumstick,1 piece,128\nChicken Joy - Thigh,1 piece,386\nOne-piece Burger Steak with rice,1 portion,305\nChocolate Sundae,1 serving,270\nRegular crispy fries,1 serving,340\n\nManually added,Portion,Calories (kcal)\ntesting food 123,1 plate,700.000000\ntesting food another,1 plate,10000.000000\ntest,test,34.000000\n\0";
    fptr = fopen("meals.csv", "w");
    
    if (fptr == NULL) {
        printf("The file is not opened, there is an error\n");
        exit(1);
    }
    fprintf(fptr, "%s", writeText);
    fclose(fptr);
}

void FileRead(char *searchterm, Food **structs, int *memAllocated, int *numElements) {
    strScanf("What food do you want to search for? (Can be an incomplete name, just be below 127 characters): ", searchterm);

    FILE* fptr;
    char currentLine[255];

    fptr = fopen("meals.csv", "r");
    
    if (fptr == NULL) {
        printf("File does not exist, creating file... \n\n");
        // exit(1);
        createMealsFile();
        fptr = fopen("meals.csv", "r");
    }

    printf("Searching file...\n");
    
    char isHeaderLine = 'y'; // because first line is a header
    char currentHeader[30];

    int count = 0; // This is used to count the number of lines
    
    // gets a line from fptr every time the while loop runs, stops when it reaches EOF
    // the line is 1000 bytes long
    while (fgets(currentLine, 255, fptr) != NULL) {
        
        // CURRENT LINE is a header
        if (isHeaderLine == 'y') {
            sscanf(currentLine, " %[^,]", currentHeader); // %[^,] reads until the first comma
            // printf("------------%s------------\n", currentHeader);
            isHeaderLine = 'n';

        // CURRENT LINE IS A \n, next line is a Header
        } else if (strcmp(currentLine, "\n") == 0) {

            isHeaderLine = 'y'; // The next iteration 

        // CURRENT LINE IS A standard record
        } else {

            Food currentFood;
            // %[^,] is used to read something until a comma is encountered - a type of regex
            int fieldsParsed = sscanf(currentLine, " %[^,], %[^,], %f ", currentFood.name, currentFood.portion, &currentFood.calories);
            if (currentLine[0] == '\"') { // If it starts with quotation marks
                sscanf(currentLine, "\"%[^\"]\",%[^,], %f", currentFood.name, currentFood.portion, &currentFood.calories);

            } else if (fieldsParsed != 3) { // portion field is missing
                
                strcpy(currentFood.portion, "-");
                sscanf(currentLine, " %[^,], , %f ", currentFood.name, &currentFood.calories);
                
            }
            // printf("food name is \"%s\", search term is \"%s\", strstr is %s \n", currentFood.name, searchterm, strstr(currentFood.name, searchterm));

            
            // This part makes a lowercase version of the current variable
            char lowerName[127];
            char lowerSearchTerm[127];

            strcpy(lowerName, currentFood.name);
            strcpy(lowerSearchTerm, searchterm);
            toLowerCase(lowerName);
            toLowerCase(lowerSearchTerm);


            // Checks if the search term is a substring of the name of the food in the current line
            if (strstr(lowerName, lowerSearchTerm) != NULL) {
                currentFood.lineNumber = count;
                strcpy(currentFood.category, currentHeader);
                // printf("%s", currentFood.category);
                appendFood(structs, memAllocated, numElements, currentFood);
            }
        }
        // printf("File is currently at line %d", count);
        count++;
    }
    // prints all relevant search results
    if (*numElements > 0) {
        int i = 0;
        printf("----------SEARCH RESULTS----------\n");
        printf("------------------------------------------------------------------------------------------------------\n");
        printf("| Num | Food Item                                       | Portion    | Calories | Category           |\n");
        printf("|-----|-------------------------------------------------|------------|----------|--------------------|\n");
        while (i < *numElements) {
            // prints the table, aligns everything to the left while taking the minimum amount of spaces as stated by the integer
            printf("| %-3d | %-47s | %-10s | %-8.2f | %-18s |\n", i+1, (*structs)[i].name, (*structs)[i].portion, (*structs)[i].calories, (*structs)[i].category);
            i++;
        }
    } else {
        printf("Sorry, no results have been found.\n");
    }
    printf("------------------------------------------------------------------------------------------------------\n");

    fclose(fptr);
    // free(mealIDs.nums);

    // printf("The file is now closed :D\n");
}

// going up one line, clearing that line, then going to the start 2 times
void moveCursor(int num) {
    int i = 0;
    while (i < num) {
        printf("\x1b[A \x1b[2K \r");
        i++;
    }
}

// The reason why I used a double pointer is so that reallocation need not be done within one function, it can be done in others
// reallocation requires assigning of variables, but i dont want to make this struct a global variable
void appendFood(Food **structs, int *memAllocated, int *numElements, Food food) {

    if (*numElements == *memAllocated) {
        // I double the length of the array if the current array is filled
        *memAllocated *= 2;

        // reallocates the array of pointers
        Food *temp = (Food*)realloc(*structs, *memAllocated * sizeof(Food));
        if (temp == NULL) {
            printf("Reallocation Failed\n");
            exit(1);
        } 
        *structs = temp;

    }
    
    // copies information into the struct itself
    (*structs)[*numElements] = food;    

    (*numElements)++; // incrementing the number of elements
}

void initMeal(Meal *meal) {
    // calloc for Food[]
    int memAllocated = 4;
    int numElements = 0;
    // I use calloc here over malloc because im not accessing some of these elements before writing to it
    meal->food = (Food*)calloc(memAllocated, sizeof(Food)); // 4 Food structs in this array of pointers

    if (meal->food == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    meal->memAllocated = 4;
    meal->numElements = 0;

}

// TODO: Make this a helper function and not put it outside when merging it
void toLowerCase(char *str) {
    // loops until the null character since the null character evaluates to false
    while (*str) {
        *str = tolower(*str);
        str++;
    }
}

// Outputs the current meal, returns the sum of the calories within this
int outputCurrentMeal(Meal currentMeal) {
    printf("------------------------------------------------------------------------------------------------\n");
    printf("| Food Item                                       | Portion    | Calories | Category           |\n");
    printf("|-------------------------------------------------|------------|----------|--------------------|\n");
    if (currentMeal.numElements == 0) {
        printf("|  -                                              |  -         |  0.00    |  -                 |\n");
        printf("------------------------------------------------------------------------------------------------\n");
        return 0;
    } else {
        int totalCalories = 0;
        int i = 0;
        while (i < currentMeal.numElements) {
            // printf("Breakfast - %d kcal\n", breakfastMeal.food[i]);
            // printf("| %-9s | %-47s | %-10s | %-8.2f |\n", mealName, currentMeal.food[i].name, currentMeal.food[i].portion, currentMeal.food[i].calories);
            printf("| %-47s | %-10s | %-8.2f | %-18s |\n", currentMeal.food[i].name, currentMeal.food[i].portion, currentMeal.food[i].calories, currentMeal.food[i].category);

            totalCalories += currentMeal.food[i].calories;
            i++;
        }
        printf("------------------------------------------------------------------------------------------------\n");
        return totalCalories;
    }
}

void outputCalorieIntake(TodayMeals todayMeals, CalorieAim calorieAim, char *currentDate) {
    
    int i;
    printf(ANSI_COLOR_RESET "\n---------------------------------------------------------------------------\n");
    // TODO: find a way to make this have the date
    printf("%*s\n", 49, "CALORIE INTAKE FOR THE DAY");
    printf("---------------------------------------------------------------------------\n\n");
    // printf("---------------------------------------------------------------------------\n");
    // printf("| Food Item                                       | Portion    | Calories |\n");
    // printf("|-------------------------------------------------|------------|----------|\n");

    printf("%*s\n", 33+9, "BREAKFAST");
    int breakfastCalories = outputCurrentMeal(todayMeals.breakfast);

    printf("%*s\n", 35+5, "LUNCH");
    int lunchCalories = outputCurrentMeal(todayMeals.lunch);

    printf("%*s\n", 35+6, "DINNER");
    int dinnerCalories = outputCurrentMeal(todayMeals.dinner);

    printf("%*s\n", 35+6, "SNACKS");
    int snacksCalories = outputCurrentMeal(todayMeals.snacks);

    int caloriesDay = breakfastCalories + lunchCalories + dinnerCalories + snacksCalories;
    
    printf("\n-----------------------------------------------------------------------------\n");
    printf("%*s\n", 49, "CALORIE INFORMATION");
    printf("-----------------------------------------------------------------------------\n");


    printf("Total Daily Calorie Intake for the day: %d kcal\n", caloriesDay);

    // output whether user's daily intake go above/below the target intake
    int difference = caloriesDay - calorieAim.calorieTarget;
    if (caloriesDay < calorieAim.calorieTarget && calorieAim.isGainingWeight == 'y') {
        printf("You consumed " ANSI_COLOR_YELLOW_BOLD "%d kcal" ANSI_COLOR_RESET " short of your calorie target (%d kcal).\n", difference * -1, calorieAim.calorieTarget);
    } else if (caloriesDay > calorieAim.calorieTarget && calorieAim.isGainingWeight == 'y') {
        printf("You consumed " ANSI_COLOR_GREEN_BOLD "%d kcal" ANSI_COLOR_RESET " over of your calorie target (%d kcal).\n", difference, calorieAim.calorieTarget);
    } else if (caloriesDay < calorieAim.calorieTarget && calorieAim.isGainingWeight == 'n') {
        printf("You consumed" ANSI_COLOR_GREEN_BOLD " %d kcal " ANSI_COLOR_RESET "short your calorie target (%d kcal).\n", difference * -1, calorieAim.calorieTarget);
    } else if (caloriesDay > calorieAim.calorieTarget && calorieAim.isGainingWeight == 'n') {
        printf("You consumed" ANSI_COLOR_YELLOW_BOLD " %d kcal " ANSI_COLOR_RESET "over your calorie target (%d kcal).\n", difference, calorieAim.calorieTarget);
    } else {
        printf(ANSI_COLOR_CYAN "Wow! you hit your calorie target exactly!\n" ANSI_COLOR_RESET);
    }

    printf("---------------------------------------------------\n");
    
    char edit[127];
    strScanf("Enter y to edit, any other key to save: ", edit);
    if (strcmp(edit, "y") == 0) {
        
        system("cls");
        askCalorieDay(calorieAim, todayMeals, currentDate);

    } else {
        // printf("Date passed into currentDate to insert food is %s", currentDate);
        insertFoods(todayMeals, currentDate);

    }
    
}

void askCalorieDay(CalorieAim userCalorieAim, TodayMeals todayMeals, char *currentDate) {
    // printf("%d", todayMeals.snacks.numElements);
    int num;
    float breakfast;
    Meal* meals[4] =  {&todayMeals.breakfast, &todayMeals.lunch,  &todayMeals.dinner,  &todayMeals.snacks};
    int mealIndex = 5; // any number but 0-4


    while (mealIndex != 0) {
        // This is used to store what the user eats for this specific meal
        int i = 0;
        while (i < 4) {
            printf(ANSI_COLOR_CYAN"%d. "ANSI_COLOR_RESET"%s\n", i+1, mealNames[i]);
            outputCurrentMeal(*(meals[i]));
            i++;
        }

        intScanf("Which one of these meals do you want start adding on to? (Enter a number from 1 to 4, 0 to stop, " ANSI_COLOR_PURPLE "100 to CLEAR the MEAL" ANSI_COLOR_RESET ")\n" ANSI_COLOR_CYAN, &mealIndex, 0);
        while ((mealIndex < 0 || mealIndex > 4)) {
            if (mealIndex == 100) {
                break;
            }
            printf(ANSI_COLOR_RED "Invalid option. Please try again." ANSI_COLOR_CYAN);
            moveCursor(1);
            intScanf("", &mealIndex, 0);
        }
        if (mealIndex == 0) {
            break;
        } else if (mealIndex == 100)
        {
            // initialises the meals again (which effectively resets it)
            int initMealCount = 0;
            while (initMealCount < 4) {
                initMeal(meals[initMealCount]);
                initMealCount++;
            }
            system("cls");
            continue;
        }
        

        mealIndex--; // since arrays start from 0 and not from 1
        
        system("cls");
        printf(ANSI_COLOR_RESET "---------%s---------\n", mealNames[mealIndex]);

        char searchterm[127];
        int optionChosen = 0;
        // array of food options which will be returned by the FileRead() function
        Meal meal;

        while (optionChosen == 0) {
            // Initialises the meal every time when the loop runs (which is when the user wants to search again)
            initMeal(&meal);

            FileRead(searchterm, &(meal.food), &(meal.memAllocated), &(meal.numElements));
            // This is to skip straight to entering data into the database again if there were no results
            if (meal.numElements > 0) {
                intScanf("Which of these would you like to pick? (0 to search in the database again): ", &optionChosen, 0);
                int invalidCount = 0;
                while (optionChosen < 0 || optionChosen > meal.numElements) {
                    if (invalidCount > 0) {
                        moveCursor(2);
                    }
                    char errorMsg[127];
                    sprintf(errorMsg,ANSI_COLOR_RED "%d is an invalid number. Please select within the range of 0 to %d.\n" ANSI_COLOR_RESET, optionChosen, meal.numElements);
                    
                    intScanf(errorMsg, &optionChosen, 0);
                    invalidCount++;
                }

            }

        }
        // when i input a new food, when it outputs a food id onto the database that is one less than the normal
        // when getting food from the database the food id is normal
        meal.food[optionChosen-1].lineNumber++;
        // This is after user actually selects something
        appendFood(&(meals[mealIndex]->food), &(meals[mealIndex]->memAllocated), &(meals[mealIndex]->numElements), meal.food[optionChosen-1]);

        mealIndex++;
        system("cls");
    }
    outputCalorieIntake(todayMeals, userCalorieAim, currentDate);

    free(meals[0]->food);
    free(meals[1]->food);
    free(meals[2]->food);
    free(meals[3]->food);

}
void mainMenuPrompt(void) {
    // Main menu portion
    system("cls");
    printf(ANSI_COLOR_RESET "---------------------------------------------------\n");
    printf("                    MAIN MENU\n");
    printf("---------------------------------------------------\n");
    printf("What would you like to do today?\n");
    printf("1. Exit\n");
    printf("2. View calorie intake information\n");
    printf("3. Create a new meal template\n");
    printf("4. Search the current foods\n---------------------------------------------------\n");
}

void createHistoryFile(void) {
    FILE *fptr;
    fptr = fopen("history.csv", "w");
    
    if (fptr == NULL) {
        printf("The file is not opened, there is an error\n");
        exit(1);
    }
    fprintf(fptr, "CalorieTarget,isGainingWeight\n000000,n\n\nDate,Meal Type,Food ID\n");
    fclose(fptr);
}

CalorieAim getCalorieTarget(void) {
    FILE *fptr;
    fptr = fopen("history.csv", "r");
    
    if (fptr == NULL) {
        printf("The file does not exist, creating file...\n\n");

        createHistoryFile();
        fptr = fopen("history.csv", "r");
    } 
    char currentLine[255];
    if (fgets(currentLine, 255, fptr) != NULL) {
        if (fgets(currentLine, 255, fptr) != NULL) {
            CalorieAim userCalorieAim;
            sscanf(currentLine, "%d,%c", &userCalorieAim.calorieTarget, &userCalorieAim.isGainingWeight);
            fclose(fptr);
            return userCalorieAim;
        }
    }
    
}

void writeCalorieAim(CalorieAim userCalorieAim) {
    FILE *fptr;
    char currentLine[127];
    long pos;


    // Open the file in read and write mode
    fptr = fopen("history.csv", "r+");
    
    if (fptr == NULL) {
        perror("Failed to open file");
        exit(1);
    }

    if (fgets(currentLine, sizeof(currentLine), fptr) != NULL) {
        // Get the current position with respect to starting of the file.(which is after the first line)
        pos = ftell(fptr);
        
        // Move the file pointer to the position after the first newline
        fseek(fptr, pos, SEEK_SET);
        
        // Write data to the second line
        char data[15];
        // The %06d is so that when writing to the file a big number the file stays as is
        // and it doesnt override anything on other lines
        sprintf(data, "%06d,%c\n", userCalorieAim.calorieTarget, userCalorieAim.isGainingWeight);
        fwrite(data, sizeof(char), strlen(data), fptr);
    } else {
        perror("Failed to read the first line");
        fclose(fptr);
        exit(1);
    }

   fclose(fptr);
    
}


void initFoodHistoryItemArr(FoodHistoryItemArr *arr) {
    arr->items = (FoodHistoryItem*)calloc(4, sizeof(FoodHistoryItem)); // 4 FoodHistoryItems in this array of pointers
    if (arr->items == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    arr->memAllocated = 4;
    arr->numElements = 0;
}

void appendFoodHistoryItem(FoodHistoryItem **arr, int *memAllocated, int *numElements, FoodHistoryItem item) {
    if (*memAllocated == *numElements) {
        // I double the length of the array if the current array is filled
        *memAllocated *= 2;

        // reallocates the array of pointers
        FoodHistoryItem *temp = (FoodHistoryItem*)realloc(*arr, *memAllocated * sizeof(FoodHistoryItem));
        if (temp == NULL) {
            printf("Reallocation Failed\n");
            exit(1);
        } 
        *arr = temp;

    }
    
    // copies information into the struct itself
    (*arr)[*numElements] = item;

    (*numElements)++; // incrementing the number of elements
}

FoodHistoryItemArr searchFoodInfoAtDate(char *date) {
    FILE *fptr;
    fptr = fopen("history.csv", "r");
    
    if (fptr == NULL) {
        printf("The file is not opened, there is an error\n");
        exit(1);
    }
    char currentLine[127];

    FoodHistoryItemArr foodHistoryItemArr;
    initFoodHistoryItemArr(&foodHistoryItemArr);

    // TODO: use %-10s to shift the thing left for the mealID
    while (fgets(currentLine, sizeof(currentLine), fptr) != NULL) {
        FoodHistoryItem foodHistoryItem;

        sscanf(currentLine, "%[^,] , %d ,%d", foodHistoryItem.date, &foodHistoryItem.mealType, &foodHistoryItem.foodId);
        if (strcmp(foodHistoryItem.date, date) == 0) {
            appendFoodHistoryItem(&foodHistoryItemArr.items, &foodHistoryItemArr.memAllocated, &foodHistoryItemArr.numElements, foodHistoryItem);
        }
    
    }
    return foodHistoryItemArr;

}

Food getFoodFromID(int num) {
    if (num == 0) {
        // return something that represents nothing
    }
    FILE *fptr;
    fptr = fopen("meals.csv", "r");
    
    if (fptr == NULL) {
        printf("The file is not opened, there is an error\n");
        exit(1);
    }
    char currentLine[127];
    char isHeaderLine = 'y'; // because first line is a header
    char currentHeader[30];
    // is 0 because counting starts from 0 on the other end and
    // that one terminates late so this one must terminate late too
    int count = 0;

    while (count < num) {
        fgets(currentLine, sizeof(currentLine), fptr);
        if (isHeaderLine == 'y') {
            sscanf(currentLine, " %[^,]", currentHeader); // %[^,] reads until the first comma
            isHeaderLine = 'n';

        // CURRENT LINE IS A \n, next line is a Header
        } else if (strcmp(currentLine, "\n") == 0) {

            isHeaderLine = 'y'; // The next iteration 

        // CURRENT LINE IS A standard record
        }
        count++;
    }
    Food currentFood;
    // %[^,] is used to read something until a comma is encountered - a type of regex
    int fieldsParsed = sscanf(currentLine, " %[^,], %[^,], %f ", currentFood.name, currentFood.portion, &currentFood.calories);
    if (currentLine[0] == '\"') { // If it starts with quotation marks
        sscanf(currentLine, "\"%[^\"]\",%[^,], %f", currentFood.name, currentFood.portion, &currentFood.calories);

    } else if (fieldsParsed != 3) { // portion field is missing
        
        strcpy(currentFood.portion, "-");
        sscanf(currentLine, " %[^,], , %f ", currentFood.name, &currentFood.calories);
        
    }
   
    
    currentFood.lineNumber = count;
    strcpy(currentFood.category, currentHeader);

    return currentFood;
    
}

TodayMeals getMealsFromDate(char currentDate[11]) {

    TodayMeals todaysHistoryMeals;
    Meal* historyMeals[] =  {&todaysHistoryMeals.breakfast, &todaysHistoryMeals.lunch,  &todaysHistoryMeals.dinner,  &todaysHistoryMeals.snacks};
    int mealIndex = 0;
    while (mealIndex < 4) {
        initMeal(historyMeals[mealIndex]);
        mealIndex++;
    }
    
    // FoodHistoryItemArr historyItems = searchFoodInfoAtDate("2024-07-21");
    // contains all the food history items at a specific date
    FoodHistoryItemArr historyItems = searchFoodInfoAtDate(currentDate);

    char editCalorieIntake = 'y';
    if (historyItems.numElements > 0) {
       // do something to convert this into a todayMeals object
        int currentHistory = 0;
        // iterates through historyItems
        while (currentHistory < historyItems.numElements) {
            // If the person didnt skip the meal
            if (historyItems.items[currentHistory].foodId > 0) {
                Food currentFood = getFoodFromID(historyItems.items[currentHistory].foodId);
                FoodHistoryItem historyItem = historyItems.items[currentHistory];
                // breakfast, lunch, dinner, or snacks
                int mealType = historyItem.mealType;
                appendFood(&(historyMeals[mealType]->food), &(historyMeals[mealType]->memAllocated), &(historyMeals[mealType]->numElements), currentFood);
            } else {
                // initialising the values for currentFood
                Food currentFood;
                currentFood.calories = 0.0;
                strcpy(currentFood.name, "-");
                strcpy(currentFood.portion, "-");
                strcpy(currentFood.category, "-");
                currentFood.lineNumber = 0;

                FoodHistoryItem historyItem = historyItems.items[currentHistory];
                int mealType = historyItem.mealType;
                appendFood(&(historyMeals[mealType]->food), &(historyMeals[mealType]->memAllocated), &(historyMeals[mealType]->numElements), currentFood);

            }
            
            currentHistory++;
        }
    //    printf("Do you want to edit today's calorie intake? (y/n)");
    }
    return todaysHistoryMeals;
}

void createNewFood(void) {
    Food newFood;
    char validInput = 'n';
    char foodName[127];
    while (validInput == 'n') {
        printf("What is the name of your new food? (Less than 40 characters, NO COMMAS)\n");
        
        strScanf("Examples: Nasi Lemak, Iced Milo (reg), etc.\n", foodName);
        // scanf(" %39[^\n]", newFood.name);  // Read up to 39 characters or until newline
        // Validate name input, strchr finds the first occurrence of a character in a string
        // TODO: check the current database to see if the name is taken

        if (strchr(foodName, ',') != NULL) {
            printf("Invalid input: Name contains comma.\n");
        
        } else if (!isAsciiString(foodName)) {
            printf("Invalid input: Name contains invalid characters.\n");
        } else if (strlen(foodName) > 39) {
            printf("Invalid input: Name is 40 characters and above.\n");
        } else {
            validInput = 'y';
        }
    }
    strcpy(newFood.name, foodName);
    
    char foodPortion[127];
    
    validInput = 'n';
    while (validInput == 'n') {
        printf("What is the portion unit for this? (Less than 10 characters, NO COMMAS)\n");
        strScanf("Examples: 1 pack, 20 pcs, 1 glass, 1 plate, etc.\n", foodPortion);
        
        // scanf(" %9[^\n]", newFood.portion);
        // if you want to input validate you can only put ascii characters  + length check
        if (strchr(foodPortion, ',') != NULL) {
            printf("Invalid input: Name contains commas.\n");
        } else if (!isAsciiString(foodPortion)) {
            printf("Invalid input: Name contains invalid characters.\n");
        } else if (strlen(foodPortion) > 9) {
            printf("Invalid input: Name is 10 characters and above.\n");
        } else {
            validInput = 'y';
        }
    }
    strcpy(newFood.portion, foodPortion);
    
    int calories;

    intScanf("How many calories is this? Enter an integer (in kcal): \n", &calories, 0);
    newFood.calories = (float)calories;

    FILE* fptr;
    fptr = fopen("meals.csv", "a");
    if (fptr == NULL) {
        perror("Error opening file");
        exit(1);
    }

    char writeText[255];
    sprintf(writeText, "%s,%s,%f\n", newFood.name, newFood.portion, newFood.calories);

    fwrite(writeText, sizeof(char), strlen(writeText), fptr);

    fclose(fptr);
    printf("\n\n-------------------------------------------------------------\n");
    printf("Data stored successfully!\n");
    strScanf("Press any key then press enter to continue to the main menu:", cont);

}

int isAsciiString(const char *str) {
    // Loops until the null terminator of the string
    while (*str) {
        // These if it is not a printable ascii characters
        if ((int)*str < 32 || 126 < (int)*str) {
            return 0;

        }
        str++;
    }
    // printf("char is %c, ", *str);
    return 1;
}


// makes sure a string is gotten from input safely
void strScanf(char *question, char *var) {
    int bufferLength = 126;
    printf("%s", question);
    // added 1 to account for null character at the end
    char inputBuffer[bufferLength+1];
    // while ( getchar() != '\n'); // clear current thing

    fgets(inputBuffer, bufferLength, stdin);
    // printf("%s, %d", inputBuffer, isAsciiString(inputBuffer) );
    // if the last chraracter in the inputBuffer isnt \n
    // if (strlen(inputBuffer) == 0) {
    //     printf("Invalid input, you did not enter anything.\n");
    //     // recursive call to prompt user again
    //     strScanf(question, var);
    // }
    // if the thing at the end of the current string is not a \n (not the null character)
    if (inputBuffer[strlen(inputBuffer) -1] != '\n') {
        int dropped = 0;
        while (fgetc(stdin) != '\n') {
            dropped++;
        }

        // if they input exactly (bufferLength - 1) characters, there's
        // only the \n in the input buffer to flush?
        if (dropped > 0) {
            printf(ANSI_COLOR_RED "Invalid input, your input ic currently %d characters which is over the limit. try again!\n" ANSI_COLOR_RESET, dropped + 125);
            // recursive call to prompt user again
            strScanf(question, var);
        }
    } else {
        inputBuffer[strlen(inputBuffer) -1] = '\0';
        if (!isAsciiString(inputBuffer)) {
            printf(ANSI_COLOR_RED "Invalid input, your input has invalid characters\n" ANSI_COLOR_RESET);
            // recursive call to prompt user again
            strScanf(question, var);
        }
        // didnt use %s so that it includes the whole line
        // %s doesnt work properly due to spaces being possible
        sscanf(inputBuffer, "%[^\n]", var);    
    }
    
}

int isInt(char* str) {
    while (*str) {
        // These if it is not a printable ascii characters
        if (!strchr("0123456789", *str)) {
            // printf("%s", str);
            return 0;
        }
        str++;
    }
    return 1;
}

void intScanf(char *question, int *var, int repeated) {
    char currentstr[126];
    strScanf(question, currentstr);
    if (!isInt(currentstr)) {
        if (repeated == 1) moveCursor(1);
        moveCursor(1);
        printf(ANSI_COLOR_RED "Your input was not an integer. Try again.\n" ANSI_COLOR_RESET);
        // recursive call to prompt user again
        intScanf(question, var, 1);
    } else {
        sscanf(currentstr, "%d", var);     
    }
}

void outputTable(TodayMeals *weekMeals, char *dates[], int numDates) {
    int i = 0;
    printf("------------------------------------------\n");
    printf("| Num  | Date(YYYY-MM-DD) | Calories     |\n");
    printf("|------|------------------|--------------|\n");
    // char *datesShortened[numDates]; // numDates dates, each date being 6 chars in length

    i = 0;
    while (i < numDates) {
        // dates[mealCount] = (char *)malloc(11 * sizeof(char)); // Allocate 6 bytes for each string
        // strncpy(datesShortened[i], dates[i]+5, 6);
        // datesShortened[i][5] = '\0';

        TodayMeals todayMeals = weekMeals[i];
        Meal todayMealsArr[4] = {todayMeals.breakfast, todayMeals.lunch, todayMeals.dinner, todayMeals.snacks};

        int todayCalories = 0;
        int j = 0;
        while (j < 4) {
            int k = 0;
            while (k < todayMealsArr[j].numElements) {
                todayCalories += todayMealsArr[j].food[k].calories;
                k++;
            }
            j++;
        }
        // printf("%d", todayCalories);
        printf("|" ANSI_COLOR_CYAN " %-3d  " ANSI_COLOR_RESET "| %s       | %-5d  kcal  |\n", i+1 ,dates[i], todayCalories);
        i++;
    }

    printf("------------------------------------------\n");
}

void searchPastCalorieIntake(CalorieAim userCalorieAim) {
    int numDates = 7;
    intScanf(ANSI_COLOR_RESET "Enter the number of past days you want to retrieve records for (Enter 0 to return to main menu): " ANSI_COLOR_CYAN, &numDates, 0);

    TodayMeals weekMeals[numDates];
    char dateStr[11]; // date string at each stage of the loop
    char *dates[numDates]; // numDates number of dates, each date being 6 chars in length
    while (numDates != 0) {
        system("cls");
        printf(ANSI_COLOR_RESET "\n--------------------------------------\n    INFO FOR THE PAST %d DAYS\n--------------------------------------\n\n", numDates);
        time_t now = time(NULL); // Get the current calendar time (seconds since Epoch)
        struct tm *tm = localtime(&now); // converts it to local time format
        // weekMeals[0] is current date, at index 1 is yesterday, etc.
        int mealCount = 0;
        // weekMeals[0] = mealsAtDate;
        while (mealCount < numDates) {
            dates[mealCount] = (char *)malloc(11 * sizeof(char)); // Allocate 6 bytes for each string
            if (dates[mealCount] == NULL) {
                printf("Failed to allocate memory");
                exit(1);
            }
            // printf("this is before mktime\n");
            mktime(tm); // Normalise the time structure and correct any errors
            strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", tm); // Format the date as YYYY-MM-DD
            
            // printf("todays's date was: %s\n", dateStr);                     
            weekMeals[mealCount] = getMealsFromDate(dateStr);
            strcpy(dates[mealCount], dateStr);
            mealCount++;
            tm->tm_mday -= 1; // minusing 1 from the day var

        }

        outputTable(weekMeals, dates, numDates);
        int daySelect;
        intScanf("Enter the number of the one you want to select (Enter 0 to go back to main menu): \n" ANSI_COLOR_CYAN, &daySelect, 0);
        if (daySelect == 0) break;
        system("cls");

        now = time(NULL); // Get the current calendar time (seconds since Epoch)
        tm = localtime(&now); // converts it to local time format
        tm->tm_mday = tm->tm_mday - (daySelect - 1); // minusing 1 from the day var
        mktime(tm); // Normalise the time structure and correct any errors
        strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", tm); // Format the date as YYYY-MM-DD

        TodayMeals dateMeals = getMealsFromDate(dateStr);
        outputCalorieIntake(dateMeals, userCalorieAim, dateStr);
        // strScanf("Type anything to continue: ", cont);

    }
    int mealCount = 0;
    while (mealCount < numDates) {
        free(dates[mealCount]);
        mealCount++;
    }

}

time_t parse_date(const char *date_str) {
    struct tm date_tm;
    int year, month, day;
    sscanf(date_str, "%d-%d-%d", &year, &month, &day);

    date_tm.tm_year = year - 1900; // tm_year is the number of years since 1900
    date_tm.tm_mon = month - 1;    // tm_mon is zero-based (0 = January)
    date_tm.tm_mday = day;
    date_tm.tm_hour = 0;
    date_tm.tm_min = 0;
    date_tm.tm_sec = 0;
    date_tm.tm_isdst = 0;
    
    return mktime(&date_tm);
}

// Function to insert a line into a file
void insertFoods(TodayMeals todayMeals, char *dateToOverride) {
    FILE *fptr = fopen("history.csv", "r");
    if (fptr == NULL) {
        perror("Failed to open the file for reading");
        exit(1);
    }

    // Create a temporary file to store updated records
    FILE *tempFile = fopen("temp.csv", "w");;
    if (tempFile == NULL) {
        perror("Failed to create a temporary file");
        fclose(fptr);
        exit(1);
    }

    char currentLine[31];
    char previousDate[31] = "1970-01-01"; // earliest possible date
    char hasOverriden = 'n';

    int i = 0, j = 0;
    Meal meals[4] = {todayMeals.breakfast, todayMeals.lunch,  todayMeals.dinner,  todayMeals.snacks};

    // printf("date to override - %s\n\n", dateToOverride);
    // reads the first 4 lines
    fgets(currentLine, 31, fptr);
    fprintf(tempFile, "%s", currentLine);
    fgets(currentLine, 31, fptr);
    fprintf(tempFile, "%s", currentLine);
    fgets(currentLine, 31, fptr);
    fprintf(tempFile, "%s", currentLine);
    fgets(currentLine, 31, fptr);
    fprintf(tempFile, "%s", currentLine);
    // printf("date to override - %s\n\n", dateToOverride);
    // Copy the file content to the temporary file and insert the new line
    while (fgets(currentLine, 31, fptr) != NULL) {
        // if currentLine is the one to override
        char currentDate[15];
        strncpy(currentDate, currentLine, 10);
        currentDate[10] = '\0'; 
        
        // if i am at the date that i need to override, at this iteration
        // i will put all the records needed then if i encounter this date again i will skip it 
        if (strcmp(currentDate, dateToOverride) == 0) {
            if (hasOverriden == 'n') {

                while (i < 4) {
                    while (j < meals[i].numElements) {
                        char writeText[200];

                        sprintf(writeText, "%s,%d,%05d\n", currentDate, i, meals[i].food[j].lineNumber);
                        // printf("%s", writeText);
                        // NOTE: line number of 0 means food doesnt exist
                        fwrite(writeText, sizeof(char), strlen(writeText), tempFile);
                        j++;
                    }
                    i++;
                    j = 0;
                }

                hasOverriden = 'y';
            }
        } else {

            // first argument must be bigger
            double diff1 = difftime(parse_date(currentDate), parse_date(dateToOverride)) / 60 / 60 / 24;
            double diff2 = difftime(parse_date(dateToOverride), parse_date(previousDate)) / 60 / 60 / 24;
            // printf("diff 1: %lf, diff 2: %lf\n", diff1, diff2);
            
            // if the "date to override" is in between the current date and the previous date
            // in other words the date does not exist
            if (diff1 > 0.0 && diff2 > 0.0 && hasOverriden == 'n') {
                while (i < 4) {
                    while (j < meals[i].numElements) {
                        char writeText[200];
                        sprintf(writeText, "%s,%d,%05d\n", dateToOverride, i, meals[i].food[j].lineNumber);

                        // NOTE: line number of 0 means food doesnt exist
                        fwrite(writeText, sizeof(char), strlen(writeText), tempFile);
                        j++;
                    }
                    i++;
                    j = 0;
                }
                hasOverriden = 'y';
            }
            fprintf(tempFile, "%s", currentLine);
            // printf("wrote %s", currentLine);
        }
        strcpy(previousDate, currentDate);
    }

    // Close the original file (the one in read mode)
    if (hasOverriden == 'n') {
        writeMealsFile(todayMeals, dateToOverride);
    } else {

        fclose(fptr);
        fclose(tempFile);

        // Open the original file in write mode to overwrite it
        fptr = fopen("history.csv", "w");
        if (fptr == NULL) {
            printf("Failed to open the file for writing");
            // fclose(tempFile);
            exit(1);
        }

        // Reset the temporary file pointer to the beginning
        tempFile = fopen("temp.csv", "r");
        if (tempFile == NULL) {
            printf("Failed to read the temporary file");
            fclose(fptr);
            exit(1);
        }

        // Copy the content from the temporary file back to the original file
        while (fgets(currentLine, sizeof(currentLine), tempFile) != NULL) {
            // printf("Copying line to original file: %s", currentLine); // Debug print
            fprintf(fptr, "%s", currentLine);
        }

    }
    // Close the files
    fclose(fptr);
    fclose(tempFile);
    remove("temp.csv");
}

