export default (array, {
  alreadySorted = false,
  alreadyCloned = false
} = {}) => {
  const values = alreadyCloned ? array : array.slice(0)
  const numbers = alreadySorted ? values : values.sort((a, b) => a - b)

  const middle = Math.floor(numbers.length / 2)
  const isEven = numbers.length % 2 === 0
  return isEven ? (numbers[middle] + numbers[middle - 1]) / 2 : numbers[middle]
}
