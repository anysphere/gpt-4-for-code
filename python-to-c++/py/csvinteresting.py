import csv
import re
import types

class CSVReader:
    def __init__(self, file_path):
        self.file_path = file_path
        self.data = []
        self.column_headings = []
        self.process_csv()

        self.predefined_column_filters = {
            'contains': self.contains_filter,
            'regex': self.regex_filter,
            'prefix-match': self.prefix_match_filter,
        }

    def process_csv(self):
        with open(self.file_path, 'r') as csv_file:
            csv_reader = csv.reader(csv_file)
            self.column_headings = next(csv_reader)
            for row in csv_reader:
                self.data.append(row)

    def summary(self):
        num_rows = len(self.data)
        num_columns = len(self.column_headings)
        return {
            'file_path': self.file_path,
            'num_rows': num_rows,
            'num_columns': num_columns,
            'column_headings': self.column_headings
        }

    """
    Useful Utils
    """
    def filter_by_columns(self, columns_to_filter):
        """
        Filters the data to only include the specified columns.

        Args:
            columns_to_filter: List of column names to filter by.

        Raises:
            ValueError: If columns_to_filter is not a list.
            ValueError: If any column name in columns_to_filter is not found in the CSV file.
        """
        # Sanity check: columns_to_filter must be a list
        if not isinstance(columns_to_filter, list):
            raise ValueError("columns_to_filter must be a list of column names.")

        # Sanity check: all column names in columns_to_filter must be found in the CSV file
        for column in columns_to_filter:
            if column not in self.column_headings:
                raise ValueError(f"Column '{column}' not found in CSV file.")

        # Filter the data to only include the specified columns
        column_indices = [self.column_headings.index(column) for column in columns_to_filter]
        self.column_headings = columns_to_filter
        self.data = [[row[index] for index in column_indices] for row in self.data]

    """
    Predefined filters
    """
    def contains_filter(self, search_term, include_columns):
        """filter out the column that contains the search term"""
        columns_to_filter = []
        for column_heading in self.column_headings:
            if search_term in column_heading:
                columns_to_filter.append(column_heading)
        return self.filter_by_columns(columns_to_filter + include_columns)

    def prefix_match_filter(self, prefix, include_columns):
        """Filters the data to only include columns that start with the specified prefix."""
        columns_to_filter = []
        for column_heading in self.column_headings:
            if column_heading.startswith(prefix):
                columns_to_filter.append(column_heading)
        self.filter_by_columns(columns_to_filter + include_columns)

    def regex_filter(self, pattern, include_columns):
        """Filters the data to only include columns that match the regex pattern."""
        columns_to_filter = []
        for column_heading in self.column_headings:
            if re.search(pattern, column_heading):
                columns_to_filter.append(column_heading)
        self.filter_by_columns(columns_to_filter + include_columns)

    """
    Column operations
    """
    def get_column(self, column_name):
        """Returns a list of values from the specified column."""
        column_index = self.column_headings.index(column_name)
        return [row[column_index] for row in self.data]

    def search_column(self, column_name, search_term):
        """Returns rows where the specified column contains the search term."""
        column_index = self.column_headings.index(column_name)
        return [row for row in self.data if search_term in row[column_index]]

    def search_column_regex(self, column_name, pattern):
        """Returns rows where the specified column matches the regex pattern."""
        column_index = self.column_headings.index(column_name)
        return [row for row in self.data if re.search(pattern, row[column_index])]

    def replace_column_values_regex(self, column_name, pattern, new_value):
        """Replaces all values in the specified column that match the regex pattern with new_value."""
        column_index = self.column_headings.index(column_name)
        for row in self.data:
            if re.search(pattern, row[column_index]):
                row[column_index] = new_value

    def add_column(self, column_name, default_value=None):
        """Adds a new column with the specified name and fills it with the default value."""
        self.column_headings.append(column_name)
        for row in self.data:
            row.append(default_value)

    def delete_column(self, column_name):
        """Deletes the specified column."""
        column_index = self.column_headings.index(column_name)
        self.column_headings.pop(column_index)
        for row in self.data:
            row.pop(column_index)

    def rename_column(self, old_name, new_name):
        """Renames the specified column."""
        column_index = self.column_headings.index(old_name)
        self.column_headings[column_index] = new_name

    """
    Custom Operations
    """
    def group_by_column_and_filter(self, column_name, filter_tuple=None, aggregation_function=None):
        """
        Groups the data by the specified column, filters the columns, and aggregates each group independently.

        Creates the following files:
            1. One CSV file per group, named after the group value.
            2. One CSV file containing the aggregated data per group.

        Args:
            column_name: The column to group by.
            filter_tuple: A tuple with the (name of the filter function, its argument).
            aggregation_function: The function to use to aggregate the values in each group. Optional.
        """

        # Sanity check: filter_tuple must be a string
        if filter_tuple is not None:
            if not isinstance(filter_tuple, tuple) or len(filter_tuple) != 2:
                raise ValueError("filter_tuple must be a tuple of length 2 to filter by.")

        # First filter the columns
        if filter_tuple is not None:
            filter_func = self.predefined_column_filters[filter_tuple[0]]
            filter_func(filter_tuple[1], [column_name])

        # Sanity check: column_name must be found in the CSV file
        if column_name not in self.column_headings:
            raise ValueError(f"Column '{column_name}' not found in CSV file.")


        # Group the data by the specified column
        column_index = self.column_headings.index(column_name)
        groups = {}
        for row in self.data:
            group_key = row[column_index]

            if group_key not in groups:
                groups[group_key] = []
            groups[group_key].append(row)

        # Sanity check: aggregation_function must be a function
        if aggregation_function is not None:
            if not isinstance(aggregation_function, types.FunctionType):
                raise ValueError("aggregation_function must be a function.")

        # Aggregate the values in each group
        aggregations = {}
        if aggregation_function is not None:
            for group_key, group_data in groups.items():
                aggregations[group_key] = aggregation_function(group_data)

        # Make a new CSV file for each group
        for group_key, group_data in groups.items():
            file_path = f"{self.file_path.split('.')[0]}_{group_key}.csv"
            with open(file_path, 'w') as csv_file:
                csv_writer = csv.writer(csv_file)
                csv_writer.writerow(self.column_headings)
                csv_writer.writerows(group_data)

        # Write the aggregated data to a CSV file
        file_path = f"{self.file_path.split('.')[0]}_aggregations.csv"
        with open(file_path, 'w') as csv_file:
            csv_writer = csv.writer(csv_file)
            csv_writer.writerow([column_name, 'Aggregation'])
            for group_key, aggregation in aggregations.items():
                csv_writer.writerow([group_key, aggregation])


if __name__ == '__main__':
    # read the data at world-cup-2022/wc_forecasts.csv

    csv_reader = CSVReader('world-cup-2022/wc_forecasts.csv')

    # print the column headings
    print(csv_reader.column_headings)

    # filter so that it contains sim
    csv_reader.group_by_column_and_filter('group', ("regex", '(sim)|(team)'), lambda x: len(x))