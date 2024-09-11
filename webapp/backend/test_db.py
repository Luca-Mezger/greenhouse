from sqlalchemy import create_engine, text

# Define the SQLite database path (use an absolute path if possible)
DATABASE_PATH = 'sqlite:///data/greenhouse.db'

# Create the SQLAlchemy engine
engine = create_engine(DATABASE_PATH, echo=True)

# Function to test database insertion with explicit commit
def test_insert():
    with engine.begin() as connection:  # Automatically commits at the end of the block
        try:
            # Insert a simple test row
            connection.execute(text("""
                INSERT INTO SensorReadings (sensor_type, value)
                VALUES ('test_sensor', 123.45)
            """))
            print("Test insert successful")
        except Exception as e:
            print(f"Error while inserting data: {str(e)}")

# Run the test
if __name__ == '__main__':
    test_insert()
