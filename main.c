#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

//struct of data we will be working on
struct features {
    double color;
    double end_time;
    double end_temp;
    double auc;
    double dev_time;
    double origin;
    double processing;
    double roastType;
};

//working with files
DIR *FD;
FILE *entry_file;

//raw training data
struct features inputData[100];
struct features testData[100];

//sizes of training and test sets
int numTrainingSets;
int numTestSets;

//functions used in data collection
int collectData(char in_dir[64], struct features *data);

void unpackFeatures(struct dirent *in_file, char in_dir[64], int fileCounter, struct features *data);

double originToDouble(char *string);

double processingToDouble(char *string);

double roastTypeToDouble(char *string);

//functions used in prediction algorithm
double neuralNetwork(double lr, double epochs);

void shuffle(int *array, size_t n);

double sigmoid(double x);

double dSigmoid(double x);

double init_weight();

int main() {
    numTrainingSets = collectData("input-data", inputData); //gathering training data
    numTestSets = collectData("test-data", testData);  // gathering test data

    double precision = neuralNetwork(0.0039, 10000); // running neural network which will return precision
    printf("precision = %f", precision);
    getchar();
    return 0;
}

//prepares playground for reading files, then calls function unpackFeatures to do the reading
int collectData(char in_dir[64], struct features *data) {
    struct dirent *in_file; //variable for getting file name

    /* Scanning the in directory */
    if (NULL == (FD = opendir(in_dir))) // error handling
    {
        fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));
        return 0;
    }

    int fileCounter = 0;
    while ((in_file = readdir(FD))) {

        //preparing files for reading data
        if (!strcmp(in_file->d_name, "."))
            continue;
        if (!strcmp(in_file->d_name, ".."))
            continue;

        //reading file to string array
        unpackFeatures(in_file, in_dir, fileCounter, data);

        fileCounter++;
    }
    return fileCounter;
}

//reads specific values from files that will become neural network features
//reading is designed on the internal structure of .alog file
void unpackFeatures(struct dirent *in_file, char in_dir[64], int fileCounter, struct features *data) {
    char dir[64];
    for (int i = 0; i < 64; i++) {
        dir[i] = in_dir[i];
    }
    char string[64];
    const char *strFrom = "/";
    strcat(dir, strFrom);
    strcat(dir, in_file->d_name);
    entry_file = fopen(dir, "rw");

    if (entry_file == NULL) // error handling
    {
        fprintf(stderr, "Error : Failed to open entry file - %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    bool foundAUC = 0;
    bool foundBeans = 0;
    //gathering data from file
    while (fscanf(entry_file, "%s", string) == 1) {
        //read GROUND COLOR from file
        if (strstr(string, "ground_color")) {
            fscanf(entry_file, "%s", string);
            int size = (int)strlen(string); //Total size of string
            string[size - 1] = '\0';
            data[fileCounter].color = strtod(string, NULL);
        }
        //read DROP TIME from file
        if (strstr(string, "DROP_time")) {
            fscanf(entry_file, "%s", string);
            int size = (int)strlen(string); //Total size of string
            string[size - 1] = '\0';
            data[fileCounter].end_time = strtod(string,NULL);

        }
        //read DROP TEMP of bean's from file
        if (strstr(string, "DROP_BT")) {
            fscanf(entry_file, "%s", string);
            int size = (int)strlen(string); //Total size of string
            string[size - 1] = '\0';
            data[fileCounter].end_temp = strtod(string,NULL);
        }
        //read AUC from file
        if (!foundAUC && strstr(string, "AUC")) {
            fscanf(entry_file, "%s", string);
            int size = (int)strlen(string); //Total size of string
            string[size - 1] = '\0';
            data[fileCounter].auc = strtod(string,NULL);
            foundAUC = true;
        }
        //read DROP TEMP of bean's from file
        if (strstr(string, "finishphasetime")) {
            fscanf(entry_file, "%s", string);
            int size = (int)strlen(string); //Total size of string
            string[size - 1] = '\0';
            data[fileCounter].dev_time = strtod(string,NULL);
        }
        //read ORIGIN, PROCESSING, ROAST DEGREE of bean's from file
        if (!foundBeans && strstr(string, "beans")) {
            fscanf(entry_file, "%s", string);
            data[fileCounter].origin = originToDouble(string);
            fscanf(entry_file, "%s", string);
            data[fileCounter].processing = processingToDouble(string);
            fscanf(entry_file, "%s", string);
            data[fileCounter].roastType = roastTypeToDouble(string);
            foundBeans = true;
        }

    }
    fclose(entry_file);
}

//functions to transform string data into double because we can't feed neural net with string values
double originToDouble(char *string) {
    char nString[strlen(string)];
    for (size_t i = 0; i < strlen(string); i++) {
        nString[i] = string[i];
    }

    strlwr((char *) nString);
    if (strstr(nString, "gwatemala"))
        return 0;
    else if (strstr(nString, "brazil"))
        return 1;
    else if (strstr(nString, "ethiopia"))
        return 2;
    else if (strstr(nString, "honduras"))
        return 3;
    else if (strstr(nString, "rwanda"))
        return 4;
    else if (strstr(nString, "kenya"))
        return 5;
    else
        return 6;
}

double processingToDouble(char *string) {
    char nString[strlen(string)];
    for (size_t i = 0; i < strlen(string); i++) {
        nString[i] = string[i];
    }

    strlwr((char *) nString);
    if (strstr(nString, "washed"))
        return 0;
    else if (strstr(nString, "natural"))
        return 1;
    else if (strstr(nString, "honey"))
        return 2;
    else if (strstr(nString, "black_honey"))
        return 3;
    else
        return 4;
}

double roastTypeToDouble(char *string) {
    char nString[strlen(string)];
    for (size_t i = 0; i < strlen(string); i++) {
        nString[i] = string[i];
    }
    strlwr((char *) nString);
    if (strstr(nString, "espresso"))
        return 0;
    else if (strstr(nString, "filtr"))
        return 1;
    else
        return 2;
}

//neural network algorithm based on algorithm implemented in below link
//https://towardsdatascience.com/simple-neural-network-implementation-in-c-663f51447547
//I've made changes such as extending algorithm to work with more features, implemented min-max normalization
//  calculated predictions and also calculated  precision of predictions
double neuralNetwork(double lr, double epochs) {
    //variables for Neural Network
    static const int numInputs = 7;
    static const int numHiddenNodes = 7;
    static const int numOutputs = 1;

    double hiddenLayer[numHiddenNodes];
    double outputLayer[numOutputs];

    double hiddenLayerBias[numHiddenNodes];
    double outputLayerBias[numOutputs];

    double hiddenWeights[numInputs][numHiddenNodes];
    double outputWeights[numHiddenNodes][numOutputs];

    //initializing weights, biases etc.
    for (int i = 0; i < numInputs; i++) {
        for (int j = 0; j < numHiddenNodes; j++) {
            hiddenWeights[i][j] = init_weight();
        }
    }
    for (int i = 0; i < numHiddenNodes; i++) {
        hiddenLayerBias[i] = init_weight();
        for (int j = 0; j < numOutputs; j++) {
            outputWeights[i][j] = init_weight();
        }
    }
    for (int i = 0; i < numOutputs; i++) {
        outputLayerBias[i] = init_weight();
    }

    //preparing data for training
    double training_inputs[numTrainingSets][numInputs];
    double training_outputs[numTrainingSets][numOutputs];
    for (int i = 0; i < numTrainingSets; i++) {
        training_inputs[i][0] = inputData[i].end_temp;
        training_inputs[i][1] = inputData[i].end_time;
        training_inputs[i][2] = inputData[i].dev_time;
        training_inputs[i][3] = inputData[i].auc;
        training_inputs[i][4] = inputData[i].origin;
        training_inputs[i][5] = inputData[i].processing;
        training_inputs[i][6] = inputData[i].roastType;
        training_outputs[i][0] = inputData[i].color;
    }

    //calculating min and max values of features to normalize data
    double minMax[numInputs + 1][2];
    for (int i = 0; i < numInputs + 1; i++) {
        minMax[i][0] = 10000;
        minMax[i][1] = 0;
    }

    //find min and max values of features and output
    for (int i = 0; i < numTrainingSets; i++) {
        for (int j = 0; j < numInputs; j++) {

            if (minMax[j][0] > training_inputs[i][j])
                minMax[j][0] = training_inputs[i][j];
            if (minMax[j][1] < training_inputs[i][j])
                minMax[j][1] = training_inputs[i][j];
        }
        //last entry is for output minMax
        if (minMax[numInputs][0] > training_outputs[i][0])
            minMax[numInputs][0] = training_outputs[i][0];
        if (minMax[numInputs][1] < training_outputs[i][0])
            minMax[numInputs][1] = training_outputs[i][0];
    }

    //normalizing data
    for (int i = 0; i < numTrainingSets; i++) {
        for (int j = 0; j < numInputs; j++) {
            if (minMax[j][1] != minMax[j][0])
                training_inputs[i][j] = (training_inputs[i][j] - minMax[j][0]) / (minMax[j][1] - minMax[j][0]);
        }
        training_outputs[i][0] =
                (training_outputs[i][0] - minMax[numInputs][0]) / (minMax[numInputs][1] - minMax[numInputs][0]);
    }


    int trainingSetOrder[numTrainingSets];
    for (int i = 0; i < numTrainingSets; i++)
        trainingSetOrder[i] = i;


//-------------NEURAL NETWORK---------------------------------------------------
// Iterate through the entire training for a number of epochs
    for (int n = 0; n < epochs; n++) {
        // As per SGD, shuffle the order of the training set
        shuffle(trainingSetOrder, numTrainingSets);
        // Cycle through each of the training set elements
        for (int x = 0; x < numTrainingSets; x++) {
            int i = trainingSetOrder[x];

            // Compute hidden layer activation
            //hiddenLayerCompute(numHiddenNodes,&hiddenLayerBias,&training_inputs,&hiddenWeights,&hiddenLayer);
            for (int j = 0; j < numHiddenNodes; j++) {
                double activation = hiddenLayerBias[j];
                for (int k = 0; k < numInputs; k++) {
                    activation += training_inputs[i][k] * hiddenWeights[k][j];
                }
                hiddenLayer[j] = sigmoid(activation);
            }

            // Compute output layer activation
            for (int j = 0; j < numOutputs; j++) {
                double activation = outputLayerBias[j];
                for (int k = 0; k < numHiddenNodes; k++) {
                    activation += hiddenLayer[k] * outputWeights[k][j];
                }
                outputLayer[j] = sigmoid(activation);
            }
            // Compute change in output weights
            double deltaOutput[numOutputs];
            for (int j = 0; j < numOutputs; j++) {
                double dError = (training_outputs[i][j] - outputLayer[j]);
                deltaOutput[j] = dError * dSigmoid(outputLayer[j]);
            }
            // Compute change in hidden weights
            double deltaHidden[numHiddenNodes];
            for (int j = 0; j < numHiddenNodes; j++) {
                double dError = 0.0f;
                for (int k = 0; k < numOutputs; k++) {
                    dError += deltaOutput[k] * outputWeights[j][k];
                }
                deltaHidden[j] = dError * dSigmoid(hiddenLayer[j]);
            }
            // Apply change in output weights
            for (int j = 0; j < numOutputs; j++) {
                outputLayerBias[j] += deltaOutput[j] * lr;
                for (int k = 0; k < numHiddenNodes; k++) {
                    outputWeights[k][j] += hiddenLayer[k] * deltaOutput[j] * lr;
                }
            }
            // Apply change in hidden weights
            for (int j = 0; j < numHiddenNodes; j++) {
                hiddenLayerBias[j] += deltaHidden[j] * lr;
                for (int k = 0; k < numInputs; k++) {
                    hiddenWeights[k][j] += training_inputs[i][k] * deltaHidden[j] * lr;
                }
            }

        }
    }
//-------------FINISH OF NEURAL NETWORK---------------------------------------------------


//predicting roast color of test data set
    //variables for predictions
    double predictions[numTestSets];
    double nodalPrediction[numHiddenNodes];
    double model_precision = 0;

    double test_inputs[numTestSets][numInputs];
    for (int i = 0; i < numTestSets; i++) {
        test_inputs[i][0] = testData[i].end_temp;
        test_inputs[i][1] = testData[i].end_time;
        test_inputs[i][2] = testData[i].dev_time;
        test_inputs[i][3] = testData[i].auc;
        test_inputs[i][4] = testData[i].origin;
        test_inputs[i][5] = testData[i].processing;
        test_inputs[i][6] = testData[i].roastType;
    }

    //calculating predictions
    for (int i = 0; i < numTestSets; i++) {
        for (int j = 0; j < numHiddenNodes; j++) {
            for (int k = 0; k < numInputs; k++) {
                nodalPrediction[j] += test_inputs[i][k] * hiddenWeights[k][j];
            }
            nodalPrediction[j] += hiddenLayerBias[j];
            nodalPrediction[j] = sigmoid(nodalPrediction[j]);
            predictions[i] += nodalPrediction[j] * outputWeights[j][0];
        }
        predictions[i] += outputLayerBias[0];
        predictions[i] = sigmoid(predictions[i]);
        predictions[i] = predictions[i] * (minMax[numInputs][1] - minMax[numInputs][0]) + minMax[numInputs][0];
        printf("Actual %f Ag, Predicted %f Ag\n", testData[i].color, predictions[i]);
        model_precision += fabs(testData[i].color - predictions[i]);
    }
    //calculating precision of the predictions (absolute mean value of difference between actual and predicted value)
    model_precision /= numTestSets;

    //return precision of model
    return model_precision;

}

//activation function
double sigmoid(double x) { return 1 / (1 + exp(-x)); }

//derivative of activation function
double dSigmoid(double x) { return x * (1 - x); }

//random initialization of weights
double init_weight() {
    time_t t;
    srand((unsigned) time(&t));
    return ((double) rand()) / ((double) RAND_MAX);
}

void shuffle(int *array, size_t n) {
    time_t t;
    srand((unsigned) time(&t));
    if (n > 1) {
        size_t i;
        for (i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int s = array[j];
            array[j] = array[i];
            array[i] = s;
        }
    }
}