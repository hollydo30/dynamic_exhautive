////////////////////////////////////////////////////////////////////////////////
// maxweight.hh
//
// Compute the set of foods that maximizes the weight in foods, within 
// a given maximum calorie amount with the dynamic programming or exhaustive search.
//
///////////////////////////////////////////////////////////////////////////////


#pragma once


#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

#include <algorithm>

// One food item available for purchase.
class FoodItem
{
	//
	public:
		
		//
		FoodItem
		(
			const std::string& description,
			double calories,
            double weight_ounces
		)
			:
			_description(description),
			_calories(calories),
            _weight_ounces(weight_ounces)
		{
			assert(!description.empty());
			assert(calories > 0);
		}
		
		//
		const std::string& description() const { return _description; }
        double calorie() const { return _calories; }
		double weight() const { return _weight_ounces; }
		
	//
	private:
		
		// Human-readable description of the food, e.g. "spicy chicken breast". Must be non-empty.
		std::string _description;
		
		// Calories; Must be positive
		double _calories;
    
    // Food weight, in ounces; most be non-negative.
		double _weight_ounces;	
};


// Alias for a vector of shared pointers to FoodItem objects.
typedef std::vector<std::shared_ptr<FoodItem>> FoodVector;


// Load all the valid food items from the CSV database
// Food items that are missing fields, or have invalid values, are skipped.
// Returns nullptr on I/O error.
std::unique_ptr<FoodVector> load_food_database(const std::string& path)
{
	std::unique_ptr<FoodVector> failure(nullptr);
	
	std::ifstream f(path);
	if (!f)
	{
		std::cout << "Failed to load food database; Cannot open file: " << path << std::endl;
		return failure;
	}
	
	std::unique_ptr<FoodVector> result(new FoodVector);
	
	size_t line_number = 0;
	for (std::string line; std::getline(f, line); )
	{
		line_number++;
		
		// First line is a header row
		if ( line_number == 1 )
		{
			continue;
		}
		
		std::vector<std::string> fields;
		std::stringstream ss(line);
		
		for (std::string field; std::getline(ss, field, '^'); )
		{
			fields.push_back(field);
		}
		
		if (fields.size() != 3)
		{
			std::cout
				<< "Failed to load food database: Invalid field count at line " << line_number << "; Want 3 but got " << fields.size() << std::endl
				<< "Line: " << line << std::endl
				;
			return failure;
		}
		
		std::string
			descr_field = fields[0],
			calories_field = fields[1],
            weight_ounces_field = fields[2]
			;
		
		auto parse_dbl = [](const std::string& field, double& output)
		{
			std::stringstream ss(field);
			if ( ! ss )
			{
				return false;
			}
			
			ss >> output;
			
			return true;
		};
		
		std::string description(descr_field);
		double calories, weight_ounces;
		if (
			parse_dbl(calories_field, calories)
			&& parse_dbl(weight_ounces_field, weight_ounces)
		)
		{
			result->push_back(
				std::shared_ptr<FoodItem>(
					new FoodItem(
						description,
						calories,
                        weight_ounces
					)
				)
			);
		}
	}

	f.close();
	
	return result;
}


// Convenience function to compute the total weight and calories in 
// a FoodVector.
// Provide the FoodVector as the first argument
// The next two arguments will return the weight and calories back to 
// the caller.
void sum_food_vector
(
	const FoodVector& foods,
	double& total_calories,
    double& total_weight
)
{
	total_calories = total_weight = 0;
	for (auto& food : foods)
	{
		total_calories += food->calorie();
        total_weight += food->weight();
	}
}


// Convenience function to print out each FoodItem in a FoodVector,
// followed by the total weight and calories of it.
void print_food_vector(const FoodVector& foods)
{
	std::cout << "*** food Vector ***" << std::endl;
	
	if ( foods.size() == 0 )
	{
		std::cout << "[empty food list]" << std::endl;
	}
	else
	{
		for (auto& food : foods)
		{
			std::cout
				<< "Ye olde " << food->description()
				<< " ==> "
				<< "; calories = " << food->calorie()
                << "Weight of " << food->weight() << " ounces"
				<< std::endl
				;
		}
		
		double total_calories, total_weight;
		sum_food_vector(foods, total_calories, total_weight);
		std::cout
			<< "> Grand total calories: " << total_calories
			<< std::endl
            << "> Grand total weight: " << total_weight << " ounces" << std::endl
			;
	}
}


// Filter the vector source, i.e. create and return a new FoodVector
// containing the subset of the food items in source that match given
// criteria.
// This is intended to:
//	1) filter out food with zero or negative weight that are irrelevant to // our optimization
//	2) limit the size of inputs to the exhaustive search algorithm since it // will probably be slow.
//
// Each food item that is included must have at minimum min_weight and 
// at most max_weight.
//	(i.e., each included food item's weight must be between min_weight
// and max_weight (inclusive).
//
// In addition, the vector includes only the first total_size food items
// that match these criteria.
std::unique_ptr<FoodVector> filter_food_vector
(
	const FoodVector& source,
	double min_weight,
	double max_weight,
	size_t total_size	//Adjusted
)
{
	
    // TODO: implement this function, then delete the return statement below
	std::unique_ptr<FoodVector> result(new FoodVector);
	
	size_t n = 0;
	
	for (auto& food : source)
	{
		
		if( (n<total_size) && (food->weight() > 0) && (food->weight() >= min_weight) && (food->weight() <= max_weight) )
		{
			
				//add food into food_vector;
				result->push_back(std::shared_ptr<FoodItem>(
					new FoodItem(
						food->description() ,
						food->calorie() ,
                        food->weight()
					)));
					
				n++;
		 }
		
	}
	
	return result;
    
    // Return to the foodItem 
    //return foodItems;
}



// Compute the optimal set of food items with dynamic programming.
// Specifically, among the food items that fit within a total_calories,
// choose the foods whose weight-per-calorie is largest.
// Repeat until no more food items can be chosen, either because we've 
// run out of food items, or run out of space.
std::unique_ptr<FoodVector> dynamic_max_weight
(
	const FoodVector& foods,
	double total_calories
)
{	
	//std::unique_ptr<FoodVector> source(new FoodVector(foods));
	//std::unique_ptr<FoodVector> best(new FoodVector);
	// print_food_vector(*todo);
    
    // TODO: implement this function, then delete the return statement below
	std::unique_ptr<FoodVector> result (new FoodVector);
	
	int n = 0;
	for (auto food : foods)
	{
		n++;
	}
	
	
	FoodVector foods_copy;

	for (auto& food : foods)
		{
			foods_copy.push_back(std::shared_ptr<FoodItem>(
					new FoodItem(
						food->description() ,
						food->calorie() ,
                        food->weight()
					)));
		}
		
		
	
	std::vector< std::vector<double> > dynamicMatrix ( n+1, std::vector<double>(total_calories+1) );
	
	for( int i = 0; i<=n; ++i )
	{
		for( int j = 0; j<=total_calories; ++j )
		{
			
			dynamicMatrix[i][j] = 0;
		}
	}
	
	
	for( int i = 1; i<=n; ++i )
	{
		for( int j = 1; j<=total_calories; ++j )
		{
			int newj = j-foods_copy[i-1]->calorie();
			
			if(newj >=0)
		{
			double include = foods_copy[i-1]->weight() + dynamicMatrix[i-1][newj];
			
			//dynamicMatrix[i][j] = dynamicMatrix[i-1][j];
			
			if ( include > dynamicMatrix[i-1][j] )
			{
				dynamicMatrix[i][j] = include;
			}
			else
			{
				dynamicMatrix[i][j] = dynamicMatrix[i-1][j];
			}
		
		}
		else
		{
			dynamicMatrix[i][j] = dynamicMatrix[i-1][j];
		}
		
		}
	}
	
	
	//trace back from last [i][j]
	
	 
	double j = total_calories;
	
	while (n > 0 && j >= 0)
{
		
	if( dynamicMatrix[n][j] != dynamicMatrix[n-1][j] )
	{
		result->push_back(std::shared_ptr<FoodItem>(
					new FoodItem(
						foods_copy[n-1]->description() ,
						foods_copy[n-1]->calorie() ,
                        foods_copy[n-1]->weight()
					)));
					
		j = j-foods_copy[n-1]->calorie();
	}
	
	n = n-1;
}
	
	
    return result;
    
    
}


// Compute the optimal set of food items with a exhaustive search algorithm.
// Specifically, among all subsets of food items, return the subset 
// whose weight in ounces fits within the total_weight one can carry and
// whose total calories is greatest.
// To avoid overflow, the size of the food items vector must be less than 64.
std::unique_ptr<FoodVector> exhaustive_max_weight
(
	const FoodVector& foods,
	double total_calorie
)
{
// TODO: implement this function, then delete the return statement below
	int n = 0;
	
	for (auto food : foods)
	{
		n++;
	}
	
	//std::vector< std::unique_ptr<FoodVector> > subsets;
	
	int total_todo = 1<<n;
	
	std::unique_ptr<FoodVector> best (new FoodVector);
	
	double total_weight_best = 0;
	
	
	for (int i = 0; i < total_todo; i++) 
	{
		//candidate = empty vector;
		std::unique_ptr<FoodVector> candidate (new FoodVector);
		
		for (int j = 0; j <= (n-1); j++)
		{
			if ( ((1<<j) & i) != 0 )
			{
				candidate->push_back(std::shared_ptr<FoodItem>(
					new FoodItem(
						foods[j]->description() ,
						foods[j]->calorie() ,
                        foods[j]->weight()
					)));
			}
		}
		
		

			double total_weight_candidate = 0;
	
			for (auto& food : (*candidate) )
			{
			total_weight_candidate += food->weight();
			}


	
			double total_calorie_candidate = 0;
	
			for (auto& food : (*candidate) )
			{
			total_calorie_candidate += food->calorie();
			}

		
		
		if ( total_calorie_candidate <= total_calorie )
		{
			if( total_weight_candidate > total_weight_best )
			{
				
				best = std::move(candidate);
				
				total_weight_best =  total_weight_candidate;
				
			}
		}
		
	
	}
	
	return best;
}
